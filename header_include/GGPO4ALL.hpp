/************************************************************************************************************
 *                                          GGPO4ALL v0.0.1
 *              Created by Ranyodh Mandur - ✨ 2025 and GroundStorm Studios, LLC. - ✨ 2009
 *
 *                                Licensed under the MIT License (MIT).
 *                           For more details, see the LICENSE file or visit:
 *                                  https://opensource.org/licenses/MIT
 *
 *                        GGPO4ALL is a free open source rollback netcode library
************************************************************************************************************/
#pragma once // :^)

//DO NOT DO "using namespace GGPO", it's bad practice and will bring in the using namespace std bit so if you do that you deserve to be miserable

namespace GGPO //I like using namespace std, but ik other ppl don't so this will limit its spread just don't do using namespace GGPO
{
    using namespace std; // >O<
}

/*
if you want to run GGPO4ALL in debug mode define:

#define GGPO_DEBUG

if you want terminal output from GGPO4ALL then you have to define:

#define GGPO_USING_CONSOLE

IMPORTANT: if you want to use console on windows, you must enable ANSI colour codes or call the EnableColours() function

before including GGPO4ALL anywhere in your code
*/

//you can change these here if u want

 //=========================================================================================== Macros ===========================================================================================//

#ifndef GGPO_MAX
#  define GGPO_MAX(x, y)        (((x) > (y)) ? (x) : (y))
#endif

#ifndef GGPO_MIN
#  define GGPO_MIN(x, y)        (((x) < (y)) ? (x) : (y))
#endif

#ifndef GGPO_ASSERT

#define GGPO_ASSERT(fp_Expr) \
        do { \
            if (not (fp_Expr)) \
            { \
                GGPO::Platform::AssertFailed(__FILE__, __LINE__, #fp_Expr, GGPO::Platform::GetProcessID()); \
            } \
        } while (not 69);

#endif

#define GGPO_DEFAULT_LOGGER_FLAGS Logger::Flags::ALL_LOGS | Logger::Flags::FLUSH_ERROR | Logger::Flags::FLUSH_FATAL | Logger::Flags::LOG_TO_ONLY_SNAPSHOT_BUFFER

#define GGPO_DEFAULT_LOG_OUTPUT_DIRECTORY "./logs"

#define GGPO_DEFAULT_RINGBUFFER_SIZE 64

#include <format>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <stdarg.h>
#include <string_view>
#include <memory.h>
#include <array>
#include <cstddef>
#include <cassert>
#include <utility>
#include <cstring>

#include <thread>

#include <cstdint>

#include <optional>
#include <limits>

#include <csignal>

// Platform-Specific Includes
#if defined(_WIN32) || defined(_WIN64)

    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN //avoid winsock and other conflicts

    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <timeapi.h> //apparently i need this idk where i took it out but i did tho lmfao


#elif defined(__linux__) || defined(__APPLE__)

    #include <time.h>
    #include <stdarg.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <stdlib.h>
    #include <errno.h>

#else

    #error Unsupported platform!

#endif

 //=========================================================================================== Main Types ===========================================================================================//

constexpr int MAX_PLAYERS = 4;
constexpr int MAX_PREDICTION_FRAMES = 8;
constexpr int MAX_SPECTATORS = 32;

constexpr int SPECTATOR_INPUT_INTERVAL = 4;

namespace GGPO
{
    using PlayerHandle = int;

    enum class PlayerType : int
    {
        Local,
        Remote,
        Spectator
    };

    /*
     * The Player structure used to describe players in ggpo_add_player
     *
     * size: Should be set to the sizeof(Player)
     *
     * type: One of the PlayerType values describing how inputs should be handled
     *       Local players must have their inputs updated every frame via
     *       ggpo_add_local_inputs.  Remote players values will come over the
     *       network.
     *
     * player_num: The player number.  Should be between 1 and the number of players
     *       In the game (e.g. in a 2 player game, either 1 or 2).
     *
     * If type == PLAYERTYPE_REMOTE:
     *
     * u.remote.ip_address:  The ip address of the ggpo session which will host this
     *       player.
     *
     * u.remote.port: The port where udp packets should be sent to reach this player.
     *       All the local inputs for this session will be sent to this player at
     *       ip_address:port.
     *
     */

    struct Player
    {
        int               size;
        PlayerType    type;
        int               player_num;

        union
        {
            struct
            {
            } local;
            struct
            {
                char           ip_address[32];
                unsigned short port;
            } remote;
        } u;
    };

    struct LocalEndpoint
    {
        int      player_num;
    };

    enum class ErrorCode : int
    {
        OK = 69,
        SUCCESS = 0,
        GENERAL_FAILURE = -1,
        INVALID_SESSION = 1,
        INVALID_PLAYER_HANDLE = 2,
        PLAYER_OUT_OF_RANGE = 3,
        PREDICTION_THRESHOLD = 4,
        UNSUPPORTED = 5,
        NOT_SYNCHRONIZED = 6,
        IN_ROLLBACK = 7,
        INPUT_DROPPED = 8,
        PLAYER_DISCONNECTED = 9,
        TOO_MANY_SPECTATORS = 10,
        INVALID_REQUEST = 11,
        FATAL_DESYNC = 12,
        NULLPTR_PASSED_AS_VALUE
    };


    consteval string_view
        ErrorToString(ErrorCode fp_ErrorCode)
    {
        switch (fp_ErrorCode)
        {
        case ErrorCode::OK: return "No error.";
        case ErrorCode::GENERAL_FAILURE: return "General failure.";
        case ErrorCode::INVALID_SESSION: return "Invalid session.";
        case ErrorCode::INVALID_PLAYER_HANDLE: return "Invalid player handle.";
        case ErrorCode::PLAYER_OUT_OF_RANGE: return "Player out of range.";
        case ErrorCode::PREDICTION_THRESHOLD: return "Prediction threshold exceeded.";
        case ErrorCode::UNSUPPORTED: return "Unsupported operation.";
        case ErrorCode::NOT_SYNCHRONIZED: return "Not synchronized.";
        case ErrorCode::IN_ROLLBACK: return "Currently in rollback.";
        case ErrorCode::INPUT_DROPPED: return "Input was dropped.";
        case ErrorCode::PLAYER_DISCONNECTED: return "Player disconnected.";
        case ErrorCode::TOO_MANY_SPECTATORS: return "Too many spectators connected.";
        case ErrorCode::INVALID_REQUEST: return "Invalid request.";
        case ErrorCode::FATAL_DESYNC: return "Fatal desynchronization detected!";
        case ErrorCode::NULLPTR_PASSED_AS_VALUE: return "Tried to pass nullptr ref!";
        default: return "Unknown  error.";
        }
    }

    constexpr inline bool
        Succeeded(ErrorCode fp_Result)
        noexcept
    {
        return fp_Result == ErrorCode::SUCCESS;
    }

    constexpr int INVALID_HANDLE = -1;

    /*
     * The EventCode enumeration describes what type of event just happened.
     *
     * EVENTCODE_CONNECTED_TO_PEER - Handshake with the game running on the
     * other side of the network has been completed.
     *
     * EVENTCODE_SYNCHRONIZING_WITH_PEER - Beginning the synchronization
     * process with the client on the other end of the networking.  The count
     * and total fields in the u.synchronizing struct of the Event
     * object indicate progress.
     *
     * EVENTCODE_SYNCHRONIZED_WITH_PEER - The synchronziation with this
     * peer has finished.
     *
     * EVENTCODE_RUNNING - All the clients have synchronized.  You may begin
     * sending inputs with ggpo_synchronize_inputs.
     *
     * EVENTCODE_DISCONNECTED_FROM_PEER - The network connection on
     * the other end of the network has closed.
     *
     * EVENTCODE_TIMESYNC - The time synchronziation code has determined
     * that this client is too far ahead of the other one and should slow
     * down to ensure fairness.  The u.timesync.frames_ahead parameter in
     * the Event object indicates how many frames the client is.
     *
     */
    enum class EventCode : int
    {
        ConnectedToPeer = 1000,
        SynchronizingWithPeer = 1001,
        SynchronizedWithPeer = 1002,
        Running = 1003,
        DisconnectedFromPeer = 1004,
        TimeSync = 1005,
        ConnectionInterrupted = 1006,
        ConnectionResumed = 1007
    };

    constexpr string_view
        EventToString(EventCode fp_EventCode)
    {
        switch (fp_EventCode)
        {
        case EventCode::ConnectedToPeer:        return "ConnectedToPeer";
        case EventCode::SynchronizingWithPeer:  return "SynchronizingWithPeer";
        case EventCode::SynchronizedWithPeer:  return "SynchronizedWithPeer";
        case EventCode::Running:                   return "Running";
        case EventCode::DisconnectedFromPeer:  return "DisconnectedFromPeer";
        case EventCode::TimeSync:                return "TimeSync";
        case EventCode::ConnectionInterrupted:  return "ConnectionInterrupted";
        case EventCode::ConnectionResumed:      return "ConnectionResumed";
        default:                                            return "UnknownEvent";
        }
    }

    /*
     * The Event structure contains an asynchronous event notification sent
     * by the on_event callback.  See EventCode, above, for a detailed
     * explanation of each event.
     */
    struct Event
    {
        Event() = default;

        EventCode code;

        union
        {
            struct
            {
                PlayerHandle  player;
            }connected;
            struct
            {
                PlayerHandle  player;
                int               count;
                int               total;
            }synchronizing;
            struct
            {
                PlayerHandle  player;
            }synchronized;
            struct
            {
                PlayerHandle  player;
            }disconnected;
            struct
            {
                int               frames_ahead;
            }timesync;
            struct
            {
                PlayerHandle  player;
                int               disconnect_timeout;
            }connection_interrupted;
            struct
            {
                PlayerHandle  player;
            }connection_resumed;
        } u;
    };

    /*
     * The NetworkStats function contains some statistics about the current
     * session.
     *
     * network.send_queue_len - The length of the queue containing UDP packets
     * which have not yet been acknowledged by the end client.  The length of
     * the send queue is a rough indication of the quality of the connection.
     * The longer the send queue, the higher the round-trip time between the
     * clients.  The send queue will also be longer than usual during high
     * packet loss situations.
     *
     * network.recv_queue_len - The number of inputs currently buffered by the
     * .net network layer which have yet to be validated.  The length of
     * the prediction queue is roughly equal to the current frame number
     * minus the frame number of the last packet in the remote queue.
     *
     * network.ping - The roundtrip packet transmission time as calcuated
     * by .net.  This will be roughly equal to the actual round trip
     * packet transmission time + 2 the interval at which you call ggpo_idle
     * or ggpo_advance_frame.
     *
     * network.kbps_sent - The estimated bandwidth used between the two
     * clients, in kilobits per second.
     *
     * timesync.local_frames_behind - The number of frames .net calculates
     * that the local client is behind the remote client at this instant in
     * time.  For example, if at this instant the current game client is running
     * frame 1002 and the remote game client is running frame 1009, this value
     * will mostly likely roughly equal 7.
     *
     * timesync.remote_frames_behind - The same as local_frames_behind, but
     * calculated from the perspective of the remote player.
     *
     */
    struct NetworkStats
    {
        struct
        {
            int   send_queue_len;
            int   recv_queue_len;
            int   ping;
            int   kbps_sent;
        } network;
        struct
        {
            int   local_frames_behind;
            int   remote_frames_behind;
        } timesync;
    };
}

//=========================================================================================== RingBuffer ===========================================================================================//

namespace GGPO {

    template<typename T, size_t pm_MaxCapacity>
    class RingBuffer
    {
        static_assert(pm_MaxCapacity > 0, "RingBuffer size must be greater than 0");

    public:
        constexpr RingBuffer() = default;

        constexpr void
            Clear()
            noexcept
        {
            pm_Head = pm_Size = 0;
        }

        [[nodiscard]] bool
            TryPush(const T& fp_Val)
        {
            if (IsFull())
            {
                return false;
            }

            PushOverwrite(fp_Val); // Safe push, now guaranteed to not assert

            return true;
        }

        [[nodiscard]] bool
            TryPush(T&& fp_Val)
        {
            if (IsFull())
            {
                return false;
            }

            PushOverwrite(move(fp_Val)); // Move into buffer

            return true;
        }

        template<typename... Args>
        [[nodiscard]] bool
            TryEmplace(Args&&... fp_Args)
        {
            if (IsFull())
            {
                return false;
            }

            EmplaceOverwrite(forward<Args>(fp_Args)...);

            return true;
        }

        constexpr void
            Push(const T& fp_Val)
        {
            PushOverwrite(fp_Val);
        }

        constexpr void
            Push(T&& fp_Val)
        {
            PushOverwrite(move(fp_Val));
        }

        template<typename... Args>
        constexpr void
            Emplace(Args&&... fp_Args)
        {
            EmplaceOverwrite(std::forward<Args>(fp_Args)...);
        }

        void
            Pop()
        {
            if (IsEmpty())
            {
                throw underflow_error("Cannot pop from empty RingBuffer");
            }

            // We remove the oldest element, oldest is at FrontIndex() so we just reduce size.
            --pm_Size;
        }

        [[nodiscard]] T&
            Front()
        {
            if (IsEmpty())
            {
                throw underflow_error("Cannot access front of empty RingBuffer");
            }

            return pm_Buffer[FrontIndex()];
        }

        [[nodiscard]] const T&
            Front()
            const
        {
            if (IsEmpty())
            {
                throw underflow_error("Cannot access front of empty RingBuffer");
            }

            return pm_Buffer[FrontIndex()];
        }

        [[nodiscard]] T&
            Back()
        {
            if (IsEmpty())
            {
                throw out_of_range("RingBuffer::Back: buffer is empty");
            }

            return pm_Buffer[BackIndex()];
        }

        [[nodiscard]] const T&
            Back()
            const
        {
            if (IsEmpty())
            {
                throw out_of_range("RingBuffer::Back: buffer is empty");
            }

            return pm_Buffer[BackIndex()];
        }

        [[nodiscard]] T&
            At(size_t fp_Index)
        {
            if (fp_Index >= pm_Size)
            {
                throw out_of_range("RingBuffer::At: index out of range");
            }

            return pm_Buffer[(FrontIndex() + fp_Index) % pm_MaxCapacity];
        }

        [[nodiscard]] const T&
            At(size_t fp_Index)
            const
        {
            if (fp_Index >= pm_Size)
            {
                throw out_of_range("RingBuffer::At: index out of range");
            }

            return pm_Buffer[(FrontIndex() + fp_Index) % pm_MaxCapacity];
        }

        [[nodiscard]] constexpr size_t
            CurrentSize()
            const noexcept
        {
            return pm_Size;
        }

        [[nodiscard]] constexpr size_t
            MaxCapacity()
            const noexcept
        {
            return pm_MaxCapacity;
        }

        [[nodiscard]] constexpr bool
            IsEmpty()
            const noexcept
        {
            return pm_Size == 0;
        }

        [[nodiscard]] constexpr bool
            IsFull()
            const noexcept
        {
            return pm_Size == pm_MaxCapacity;
        }

    private:
        array<T, pm_MaxCapacity> pm_Buffer{};
        size_t pm_Head = 0;   // index of next write
        size_t pm_Size = 0;   // number of valid elements

    private:
        ////////////////// helpers //////////////////

        [[nodiscard]] size_t
            FrontIndex() const noexcept
        {
            // Oldest element
            return (pm_Head + pm_MaxCapacity - pm_Size) % pm_MaxCapacity;
        }

        [[nodiscard]] size_t
            BackIndex() const noexcept
        {
            // Most recently inserted element
            return (pm_Head + pm_MaxCapacity - 1) % pm_MaxCapacity;
        }

        void
            AdvanceHead() noexcept
        {
            pm_Head = (pm_Head + 1) % pm_MaxCapacity;
            if (pm_Size < pm_MaxCapacity)
            {
                ++pm_Size;
            }
            // If already full, we overwrote the oldest; size stays at capacity.
        }

        void
            PushOverwrite(const T& fp_Val)
        {
            pm_Buffer[pm_Head] = fp_Val;
            AdvanceHead();
        }

        void
            PushOverwrite(T&& fp_Val)
        {
            pm_Buffer[pm_Head] = move(fp_Val);
            AdvanceHead();
        }

        template<typename... Args>
        void
            EmplaceOverwrite(Args&&... fp_Args)
        {
            pm_Buffer[pm_Head] = T(forward<Args>(fp_Args)...);
            AdvanceHead();
        }
    };

} // namespace GGPO

 //=========================================================================================== Logger ===========================================================================================//

namespace GGPO {

    constexpr uint32_t MAX_NUMBER_OF_LOGS = 1024;

#if (defined(_WIN32) || defined(_WIN64)) && defined(GGPO_USING_CONSOLE) //this needs to be called at least once at the start of the program to enable ANSI colour codes on windows consoles for pretty logging

    inline bool
        EnableColors()
    {
        DWORD f_ConsoleMode;
        HANDLE f_OutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);

        if (GetConsoleMode(f_OutputHandle, &f_ConsoleMode))
        {
            SetConsoleMode(f_OutputHandle, f_ConsoleMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            return true;
        }
        else
        {
            cout << ("Was not able to set console mode to allow windows to display ANSI escape codes") << "\n";
            return false;
        }
    }

#endif

    enum class Colours : int
    {
        Black,
        Red,
        Green,
        Yellow,
        Blue,
        Magenta,
        Cyan,
        White,

        BrightBlack,
        BrightRed,
        BrightGreen,
        BrightYellow,
        BrightBlue,
        BrightMagenta,
        BrightCyan,
        BrightWhite
    };

    [[nodiscard]] constexpr string 
        CreateColouredText
        (
            const string& fp_SampleText,
            const Colours fp_DesiredColour
        )
    {
        switch(fp_DesiredColour)
        {
            //////////////////// Regular Colours ////////////////////

            case Colours::Black: return "\x1B[30m" + fp_SampleText + "\033[0m";

            case Colours::Red: return "\x1B[31m" + fp_SampleText + "\033[0m";
            
            case Colours::Green: return "\x1B[32m" + fp_SampleText + "\033[0m";
            
            case Colours::Yellow: return "\x1B[33m" + fp_SampleText + "\033[0m";
            
            case Colours::Blue: return "\x1B[34m" + fp_SampleText + "\033[0m";
            
            case Colours::Magenta: return "\x1B[35m" + fp_SampleText + "\033[0m";
            
            case Colours::Cyan: return "\x1B[36m" + fp_SampleText + "\033[0m";
            
            case Colours::White: return "\x1B[37m" + fp_SampleText + "\033[0m";
            

            //////////////////// Bright Colours ////////////////////

            case Colours::BrightBlack: return "\x1B[90m" + fp_SampleText + "\033[0m";
            
            case Colours::BrightRed: return "\x1B[91m" + fp_SampleText + "\033[0m";
            
            case Colours::BrightGreen: return "\x1B[92m" + fp_SampleText + "\033[0m";
            
            case Colours::BrightYellow: return "\x1B[93m" + fp_SampleText + "\033[0m";
            
            case Colours::BrightBlue: return "\x1B[94m" + fp_SampleText + "\033[0m";
            
            case Colours::BrightMagenta: return "\x1B[95m" + fp_SampleText + "\033[0m";
            
            case Colours::BrightCyan: return "\x1B[96m" + fp_SampleText + "\033[0m";
            
            case Colours::BrightWhite: return "\x1B[97m" + fp_SampleText + "\033[0m";
            
            //////////////////// Just Return the Input Text Unaltered Otherwise ////////////////////

            default: return fp_SampleText;
        }
    }

    static void
        Print
        (
            const string& fp_Message,
            const Colours fp_DesiredColour = Colours::White
        )
    {
        cout << CreateColouredText(fp_Message, fp_DesiredColour) << "\n";
    }

    static void
        PrintError
        (
            const string& fp_Message,
            const Colours fp_DesiredColour = Colours::Red
        )
    {
        cerr << CreateColouredText(fp_Message, fp_DesiredColour) << "\n";
    }

    //////////////////////////////////////////////
    // LogMessage Struct
    //////////////////////////////////////////////

    struct LogMessage
    {
        string Timestamp;  // "2025-01-01 13:37:00.123"
        string Message;
        string Sender;
        uint8_t Level;

        [[nodiscard]] string
            Formatted(const string& fp_LevelName)
            const
        {
            return "[" + Timestamp + "][" + fp_LevelName + "][" + Sender + "]: " + Message;
        }
    };

    //////////////////////////////////////////////
    // Logger Class
    //////////////////////////////////////////////

    class Logger
    {
        //////////////////////////////////////////////
        // Public Destructor
        //////////////////////////////////////////////
    public:
        ~Logger() ///XXX: Just copy and pasted the flushalllogs method because they have the assert at the beginning and wont work with premature exit
        {
            for (auto& lv_LogFile : pm_LogFiles)
            {
                if (lv_LogFile.second.is_open())
                {
                    lv_LogFile.second.flush();
                }
            }  // Ensure all logs are flushed before destruction

            CloseOpenLogFiles(); //Closes any files that are open to prevent introducing vulnerabilities in privileged environments
        }

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger& operator=(Logger&&) = delete;

        /*
            needed for stack allocated Create(), allows for nrvo and also is kosher since move constructors play w the strict ownership model that is the foundation of the thread owning system uwu
            so the pattern is using optional return a nrvo Logger, and the thread thats using it calls Create() so this_thread::thread::id works properly ^_^
        */
        Logger(Logger&&) = default;

        static constexpr uint32_t FLUSH_EVERY_N_LOGS = 256u;
        static constexpr uint32_t MAX_NUMBER_OF_LOGS = 1024u;
        static constexpr uintmax_t MAX_LOG_FILE_SIZE_BYTES = 10u * 1024u * 1024u; // 10 MB

        static constexpr uint8_t FLUSH_TRACE_BIT = 1u << 0;
        static constexpr uint8_t FLUSH_DEBUG_BIT = 1u << 1;
        static constexpr uint8_t FLUSH_INFO_BIT = 1u << 2;
        static constexpr uint8_t FLUSH_WARNING_BIT = 1u << 3;
        static constexpr uint8_t FLUSH_ERROR_BIT = 1u << 4;
        static constexpr uint8_t FLUSH_FATAL_BIT = 1u << 5;

        using LogBuffer = RingBuffer<LogMessage, MAX_NUMBER_OF_LOGS>;

        //////////////////////////////////////////////
        // Protected Constructor
        //////////////////////////////////////////////
    protected:
        Logger() = default;

        ////////////////////////////////////////////////
        // Helper Enum For LogLevel Specification
        ////////////////////////////////////////////////
    public:
        enum Flags : uint32_t
        {
            // low byte is active mask
            TRACE_LOG = 1u << 0,
            DEBUG_LOG = 1u << 1,
            INFO_LOG = 1u << 2,
            WARNING_LOG = 1u << 3,
            ERROR_LOG = 1u << 4,
            FATAL_LOG = 1u << 5,

            ALL_LOGS = TRACE_LOG | DEBUG_LOG | INFO_LOG | WARNING_LOG | ERROR_LOG | FATAL_LOG,

            //middle byte is flush mask
            FLUSH_TRACE = 1u << 8,
            FLUSH_DEBUG = 1u << 9,
            FLUSH_INFO = 1u << 10,
            FLUSH_WARNING = 1u << 11,
            FLUSH_ERROR = 1u << 12,
            FLUSH_FATAL = 1u << 13,

            FLUSH_ALL = FLUSH_TRACE | FLUSH_DEBUG | FLUSH_INFO | FLUSH_WARNING | FLUSH_ERROR | FLUSH_FATAL,

            //high byte is aux flags
            DONT_CREATE_DIRECTORY = 1u << 16,
            LOG_TO_ONLY_SNAPSHOT_BUFFER = 1u << 17
        };

        //////////////////////////////////////////////
        // Protected Class Members
        //////////////////////////////////////////////
    protected:
        unordered_map<string, ofstream> pm_LogFiles;

        unique_ptr<LogBuffer> pm_SnapshotBuffer = nullptr;

        string pm_LoggerName = "No_Logger_Name";
        string pm_CurrentWorkingDirectory = "nothing";

        thread::id pm_ThreadOwnerID;

        uint8_t pm_ActiveLogMask = 0;
        uint8_t pm_FlushMask = 0;

        bool pm_LogToFile = true;

        uint32_t pm_LogSizeCounter = 0;

        //////////////////////////////////////////////
        // Public Methods
        //////////////////////////////////////////////
    public:

        [[nodiscard]] static optional<Logger>
            Create
            (
                const string& fp_DesiredLoggerName,
                const uint32_t fp_Flags,
                const string& fp_DesiredOutputDirectory = ""
            )
        {
            Logger f_CreatedLogger;

            if (not f_CreatedLogger.Initialize(fp_DesiredLoggerName, fp_DesiredOutputDirectory, fp_Flags))
            {
                PrintError("Unable to initialize logger named: " + fp_DesiredLoggerName);
                return nullopt;
            }

            return f_CreatedLogger; //NRVO
        }

        [[nodiscard]] static unique_ptr<Logger>
            CreateUnique
            (
                const string& fp_DesiredLoggerName,
                const uint32_t fp_Flags,
                const string& fp_DesiredOutputDirectory = ""
            )
        {
            unique_ptr<Logger> f_CreatedLogger(new Logger()); //this is dumb but std doesn't like my private constructor uwu!

            if (not f_CreatedLogger->Initialize(fp_DesiredLoggerName, fp_DesiredOutputDirectory, fp_Flags))
            {
                PrintError("Unable to initialize logger named: " + fp_DesiredLoggerName);
                return nullptr;
            }

            return std::move(f_CreatedLogger);
        }

        [[nodiscard]] static shared_ptr<Logger>
            CreateShared
            (
                const string& fp_DesiredLoggerName,
                const uint32_t fp_Flags,
                const string& fp_DesiredOutputDirectory = ""
            )
        {
            shared_ptr<Logger> f_CreatedLogger(new Logger()); //this is dumb but std doesn't like my private constructor uwu!

            if (not f_CreatedLogger->Initialize(fp_DesiredLoggerName, fp_DesiredOutputDirectory, fp_Flags))
            {
                PrintError("Unable to initialize logger named: " + fp_DesiredLoggerName);
                return nullptr;
            }

            return f_CreatedLogger;
        }

        bool
            UpdateThreadOwner //the owning thread must update and pass off the logger to be considered valid otherwise it wont uwu
            (
                const thread::id& fp_NewThreadID
            )
        {
            if (not AssertThreadAccess("UpdateThreadOwner"))
            {
                //can't log here since it's only triggered by improper thread usage which will trigger asserthreadacess again
                PrintError(format("Tried to call UpdateThreadOwner from a thread that didn't own logger named: {}", pm_LoggerName));
                return false;
            }

            pm_ThreadOwnerID = fp_NewThreadID;

            return true;
        }

        [[nodiscard]] bool
            UpdateActiveMask(const uint32_t fp_NewLogMask)
        {
            ////////////////////////////////////////////// Change Active Mask if logging to snapshot buffer only uwu //////////////////////////////////////////////

            if (not pm_LogToFile)
            {
                pm_ActiveLogMask = ExtractLevelMask(fp_NewLogMask);
                return true;
            }

            ////////////////////////////////////////////// flush all logs before making any changes //////////////////////////////////////////////

            if (not FlushAllLogs())
            {
                return false;
            }

            ////////////////////////////////////////////// clear every file //////////////////////////////////////////////

            pm_LogFiles.clear();

            ////////////////////////////////////////////// Reset Mask //////////////////////////////////////////////

            pm_ActiveLogMask = 0;

            ////////////////////////////////////////////// Create Log files based off of Current active mask uwu //////////////////////////////////////////////

            static const unordered_map<uint8_t, const string> f_LogLevels = //this is fine being static since its not mutable so reading from multiple threads is kosher
            {
                {TRACE_LOG, "trace.log"},
                {DEBUG_LOG, "debug.log"},
                {INFO_LOG, "info.log"},
                {WARNING_LOG, "warning.log"},
                {ERROR_LOG, "error.log"},
                {FATAL_LOG, "fatal.log"}
            };

            for (const auto& [lv_LogEnum, lv_LogStringName] : f_LogLevels)
            {
                if (fp_NewLogMask & lv_LogEnum)
                {
                    if (not CreateLogFile(pm_CurrentWorkingDirectory, lv_LogStringName))
                    {
                        PrintError("Failed to create log file named: " + lv_LogStringName);
                        return false;
                    }

                    pm_ActiveLogMask |= static_cast<uint8_t>(lv_LogEnum);
                }
            }

            ////////////////////////////////////////////// Success! //////////////////////////////////////////////

            return true;
        }

        const RingBuffer<LogMessage, MAX_NUMBER_OF_LOGS>&
            GetSnapshotBuffer()
            const noexcept
        {
            return *pm_SnapshotBuffer;
        }


        //////////////////// Flush All Logs ////////////////////

        [[nodiscard]] bool
            FlushAllLogs()
        {
            if (not AssertThreadAccess("FlushAllLogs"))
            {
                return false;
            }

            for (auto& lv_LogFile : pm_LogFiles)
            {
                if (lv_LogFile.second.is_open())
                {
                    lv_LogFile.second.flush();
                }
            }

            pm_LogSizeCounter = 0; //reset since all logs have been flushed

            return true;
        }

        [[nodiscard]] bool
            ValidateLogMsg(const uint8_t fp_LogLevel)
        {
            return (AssertThreadAccess("ValidateLogMsg") and pm_ActiveLogMask & fp_LogLevel); //return early without logging if loglevel isnt active or hasnt been initialized or if accessed from the wrong thread
        }

        //////////////////////////////////////////////////////////// Logging Functions  ////////////////////////////////////////////////////////////

        void
            Trace
            (
                const string& fp_Message,
                const string& fp_Sender
            )
        {
            if (ValidateLogMsg(static_cast<uint8_t>(Flags::TRACE_LOG))) //IMPORTANT: don't need to check if the log file was created since activelogmask tracks that as well >w< and the activemask can't be modified directly since its private
            {
                const string f_TimeStamp = GetCurrentTimestamp();
                const string f_LogEntry = "[" + f_TimeStamp + "][trace][" + fp_Sender + "]: " + fp_Message;

                pm_SnapshotBuffer->Emplace(f_TimeStamp, fp_Message, fp_Sender, static_cast<uint8_t>(Flags::TRACE_LOG));

                if (pm_LogToFile)
                {
                    ofstream& f_LogFile = pm_LogFiles.at("trace.log"); //safe to call at() here since its synced at all times w pm_ActiveMask

                    if (f_LogFile.is_open())
                    {
                        f_LogFile << f_LogEntry << "\n";

                        if (pm_LogSizeCounter++ >= FLUSH_EVERY_N_LOGS)
                        {
                            ForceFlushAllLogs(); //AssertThreadAccess is already called so this is safe UwU >O< !!!!!
                        }
                        else if (pm_FlushMask & FLUSH_TRACE_BIT)
                        {
                            f_LogFile.flush();
                        }
                    }
                }

#ifdef GGPO_USING_CONSOLE
                Print(f_LogEntry, Colours::BrightWhite);
#endif
            }
        }

        void
            Debug
            (
                const string& fp_Message,
                const string& fp_Sender
            )
        {
            if (ValidateLogMsg(static_cast<uint8_t>(Flags::DEBUG_LOG)))
            {
                const string f_TimeStamp = GetCurrentTimestamp();
                const string f_LogEntry = "[" + f_TimeStamp + "][debug][" + fp_Sender + "]: " + fp_Message;

                pm_SnapshotBuffer->Emplace(f_TimeStamp, fp_Message, fp_Sender, static_cast<uint8_t>(Flags::DEBUG_LOG));

                if (pm_LogToFile)
                {
                    ofstream& f_LogFile = pm_LogFiles.at("debug.log"); // Log to specific log file >W<

                    if (f_LogFile.is_open())
                    {
                        f_LogFile << f_LogEntry << "\n";

                        if (pm_LogSizeCounter++ >= FLUSH_EVERY_N_LOGS)
                        {
                            ForceFlushAllLogs(); //AssertThreadAccess is already called so this is safe UwU >O< !!!!!
                        }
                        else if (pm_FlushMask & FLUSH_DEBUG_BIT)
                        {
                            f_LogFile.flush();
                        }
                    }
                }

#ifdef GGPO_USING_CONSOLE
                Print(f_LogEntry, Colours::BrightBlue);
#endif
            }
        }

        void
            Info
            (
                const string& fp_Message,
                const string& fp_Sender
            )
        {
            if (ValidateLogMsg(static_cast<uint8_t>(Flags::INFO_LOG)))
            {
                const string f_TimeStamp = GetCurrentTimestamp();
                const string f_LogEntry = "[" + f_TimeStamp + "][info][" + fp_Sender + "]: " + fp_Message;

                pm_SnapshotBuffer->Emplace(f_TimeStamp, fp_Message, fp_Sender, static_cast<uint8_t>(Flags::INFO_LOG));

                if (pm_LogToFile)
                {
                    ofstream& f_LogFile = pm_LogFiles.at("info.log"); // Log to specific file and all-logs file

                    if (f_LogFile.is_open())
                    {
                        f_LogFile << f_LogEntry << "\n";

                        if (pm_LogSizeCounter++ >= FLUSH_EVERY_N_LOGS)
                        {
                            ForceFlushAllLogs(); //AssertThreadAccess is already called so this is safe UwU >O< !!!!!
                        }
                        else if (pm_FlushMask & FLUSH_INFO_BIT)
                        {
                            f_LogFile.flush();
                        }
                    }
                }

#ifdef GGPO_USING_CONSOLE
                Print(f_LogEntry, Colours::BrightGreen);
#endif
            }
        }

        void
            Warning
            (
                const string& fp_Message,
                const string& fp_Sender
            )
        {
            if (ValidateLogMsg(static_cast<uint8_t>(Flags::WARNING_LOG)))
            {
                const string f_TimeStamp = GetCurrentTimestamp();
                const string f_LogEntry = "[" + f_TimeStamp + "][warning][" + fp_Sender + "]: " + fp_Message;

                pm_SnapshotBuffer->Emplace(f_TimeStamp, fp_Message, fp_Sender, static_cast<uint8_t>(Flags::WARNING_LOG));

                if (pm_LogToFile)
                {
                    ofstream& f_LogFile = pm_LogFiles.at("warning.log"); // Log to specific file and all-logs file

                    if (f_LogFile.is_open())
                    {
                        f_LogFile << f_LogEntry << "\n";

                        if (pm_LogSizeCounter++ >= FLUSH_EVERY_N_LOGS)
                        {
                            ForceFlushAllLogs(); //AssertThreadAccess is already called so this is safe UwU >O< !!!!!
                        }
                        else if (pm_FlushMask & FLUSH_WARNING_BIT)
                        {
                            f_LogFile.flush();
                        }
                    }
                }

#ifdef GGPO_USING_CONSOLE
                Print(f_LogEntry, Colours::BrightYellow);
#endif
            }
        }

        void
            Error
            (
                const string& fp_Message,
                const string& fp_Sender
            )
        {
            if (ValidateLogMsg(static_cast<uint8_t>(Flags::ERROR_LOG)))
            {
                const string f_TimeStamp = GetCurrentTimestamp();
                const string f_LogEntry = "[" + f_TimeStamp + "][error][" + fp_Sender + "]: " + fp_Message;

                pm_SnapshotBuffer->Emplace(f_TimeStamp, fp_Message, fp_Sender, static_cast<uint8_t>(Flags::ERROR_LOG));

                if (pm_LogToFile)
                {
                    ofstream& f_LogFile = pm_LogFiles.at("error.log"); // Log to specific file and all-logs file

                    if (f_LogFile.is_open())
                    {
                        f_LogFile << f_LogEntry << "\n";

                        if (pm_LogSizeCounter++ >= FLUSH_EVERY_N_LOGS)
                        {
                            ForceFlushAllLogs(); //AssertThreadAccess is already called so this is safe UwU >O< !!!!!
                        }
                        else if (pm_FlushMask & FLUSH_ERROR_BIT)
                        {
                            f_LogFile.flush();
                        }
                    }
                }

#ifdef GGPO_USING_CONSOLE
                PrintError(f_LogEntry, Colours::Red);
#endif
            }
        }

        void
            Fatal
            (
                const string& fp_Message,
                const string& fp_Sender
            )
        {
            if (ValidateLogMsg(static_cast<uint8_t>(Flags::FATAL_LOG)))
            {
                const string f_TimeStamp = GetCurrentTimestamp();
                const string f_LogEntry = "[" + f_TimeStamp + "][fatal][" + fp_Sender + "]: " + fp_Message;

                pm_SnapshotBuffer->Emplace(f_TimeStamp, fp_Message, fp_Sender, static_cast<uint8_t>(Flags::FATAL_LOG));

                if (pm_LogToFile)
                {
                    ofstream& f_LogFile = pm_LogFiles.at("fatal.log"); // Log to specific file and all-logs file

                    if (f_LogFile.is_open())
                    {
                        f_LogFile << f_LogEntry << "\n";

                        if (pm_LogSizeCounter++ >= FLUSH_EVERY_N_LOGS)
                        {
                            ForceFlushAllLogs(); //AssertThreadAccess is already called so this is safe UwU >O< !!!!!
                        }
                        else if (pm_FlushMask & FLUSH_FATAL_BIT)
                        {
                            f_LogFile.flush();
                        }
                    }
                }

#ifdef GGPO_USING_CONSOLE
                PrintError(f_LogEntry, Colours::Magenta);
#endif
            }
        }

        //////////////////////////////////////////////
        // Protected Methods
        //////////////////////////////////////////////
    protected:
        [[nodiscard]] bool
            Initialize
            (
                const string& fp_DesiredLoggerName,
                const string& fp_DesiredOutputDirectory,
                const uint32_t fp_Flags
            )
        {
#ifdef GGPO_DEBUG
            //stringstream f_UckCPlusPlus; //XXX: cpp is a dumb fucking language sometimes holy please make good features and not dumbass nonsense holy shit
            //f_UckCPlusPlus << this_thread::get_id();
            //string f_CallerThreadID = f_UckCPlusPlus.str();

            //PrintError(format("Logger name: '{}' from thread number : {}, [Caller Thread ID]: {}", fp_DesiredLoggerName, static_cast<uint8_t>(fp_ThreadName), f_CallerThreadID));
#endif
            ////////////////////////////////////////////// Store Initializer Thread ID //////////////////////////////////////////////

            pm_ThreadOwnerID = this_thread::get_id();

            ////////////////////////////////////////////// Set Logger Name + Directory //////////////////////////////////////////////

            pm_LoggerName = fp_DesiredLoggerName;
            pm_CurrentWorkingDirectory = fp_DesiredOutputDirectory + "/" + pm_LoggerName;

            if (fp_Flags & Flags::LOG_TO_ONLY_SNAPSHOT_BUFFER)
            {
                pm_LogToFile = false;
            }

            ////////////////////////////////////////////// Set Flush Mask //////////////////////////////////////////////

            pm_FlushMask = ExtractFlushMask(fp_Flags);

            ////////////////////////////////////////////// Initialize Snapshot Ring Buffer //////////////////////////////////////////////

            pm_SnapshotBuffer = make_unique<LogBuffer>();

            ////////////////////////////////////////////// Ensure log directory exists //////////////////////////////////////////////

            if (not filesystem::exists(pm_CurrentWorkingDirectory))
            {
                if (fp_Flags & Flags::DONT_CREATE_DIRECTORY)
                {
                    PrintError("[CRITICAL_LOGGING_ERROR]: Failed to find valid log output directory");
                    return false;
                }

                try
                {
                    filesystem::create_directories(pm_CurrentWorkingDirectory); //XXX: this can throw so we wrap it in a try catch
                }
                catch (const exception& f_Exception)
                {
                    PrintError(format("Failed to create desired log output directory with exception: '{}'", f_Exception.what()));
                    return false;
                }
            }

            ////////////////////////////////////////////// Create Log Files Based on Current Active Mask //////////////////////////////////////////////

            if (not UpdateActiveMask(fp_Flags))
            {
                PrintError("[CRITICAL_LOGGING_ERROR]: Failed to create required log files for logger named: " + pm_LoggerName);
                return false;
            }

            ////////////////////////////////////////////// Success! //////////////////////////////////////////////

            return true;
        }

        //////////////////////////////////////////////////////////// Utility Functions  ////////////////////////////////////////////////////////////

        [[nodiscard]] bool
            CreateLogFile
            (
                const string& fp_FilePath,
                const string& fp_FileName
            )
        {
            ////////////////////////////////////////////// Cache Full Path String //////////////////////////////////////////////

            const string f_FullPath = fp_FilePath + "/" + fp_FileName;

            ////////////////////////////////////////////// If file exists and is too big, truncate it //////////////////////////////////////////////
            error_code f_ErrorCode;

            if (filesystem::exists(f_FullPath, f_ErrorCode) and not f_ErrorCode)
            {
                auto f_LogFileSize = filesystem::file_size(f_FullPath, f_ErrorCode);

                if (not f_ErrorCode and f_LogFileSize >= MAX_LOG_FILE_SIZE_BYTES)
                {
                    ////////////////////////////////////////////// truncate by reopening with ios::trunc //////////////////////////////////////////////

                    ofstream f_LogFile(f_FullPath, ios::out | ios::trunc);

                    if (not f_LogFile.is_open())
                    {
                        PrintError(format("Failed to truncate oversized log file: '{}' with logger named: {}", fp_FileName, pm_LoggerName));
                        return false;
                    }

                    pm_LogFiles[fp_FileName] = std::move(f_LogFile);

                    ////////////////////////////////////////////// Success! //////////////////////////////////////////////

                    return true;
                }
            }

            ////////////////////////////////////////////// If file doesn't exist or isn't too big it's business as usual UwU //////////////////////////////////////////////

            ofstream f_LogFile(f_FullPath, ios::out | ios::app);

            if (not f_LogFile.is_open())
            {
                PrintError(std::format("Failed to open log file: '{}' with logger named: {}", fp_FileName, pm_LoggerName));
                return false;
            }

            pm_LogFiles[fp_FileName] = std::move(f_LogFile);

            ////////////////////////////////////////////// Success! //////////////////////////////////////////////

            return true;
        }

        [[nodiscard]] inline string //thank you chat-gpt uwu
            GetCurrentTimestamp()
            const
        {
            const auto now = chrono::system_clock::now();
            auto time_t_now = chrono::system_clock::to_time_t(now);

            tm local_time{};

#if defined(_WIN32) || defined(_WIN64) //needa do this since localtime() isnt threadsafe uwu
            localtime_s(&local_time, &time_t_now);
#else
            localtime_r(&time_t_now, &local_time);
#endif

            stringstream f_AssembledTimeString;
            f_AssembledTimeString << put_time(&local_time, "%Y-%m-%d %H:%M:%S");

            const auto since_epoch = now.time_since_epoch();
            const auto milliseconds = chrono::duration_cast<chrono::milliseconds>(since_epoch).count() % 1000;

            f_AssembledTimeString << '.' << setfill('0') << setw(3) << milliseconds;

            return f_AssembledTimeString.str();
        }

        void
            CloseOpenLogFiles()
        {
            for (auto& _f : pm_LogFiles)
            {
                if (_f.second.is_open())
                {
                    _f.second.close();
                }
            }
        }

        [[nodiscard]] inline bool ///XXX: used for testing, this method should never call exit() for a production release, since all logging is hidden away from the game engine dev
            AssertThreadAccess(const string& fp_FunctionName) //we don't require a lock since this method guarantees only one thread is operating on any data within the Logger instance
            const
        {
            if (this_thread::get_id() == pm_ThreadOwnerID)
            {
                return true;
            }

            stringstream f_UckCPlusPlus; //XXX: cpp is a dumb fucking language sometimes holy please make good features and not dumbass nonsense holy shit
            f_UckCPlusPlus << this_thread::get_id();
            string f_CallerThreadID = f_UckCPlusPlus.str();

            PrintError(format("Logger name: '{}' called method '{}' from the wrong thread, [Caller Thread ID]: {}", pm_LoggerName, fp_FunctionName, f_CallerThreadID));

            return false;
        }

        void
            ForceFlushAllLogs() //called by functions that already do an AssertThreadAccess call in them uwu this is to avoid double calling OwO!
        {
            for (auto& lv_LogFile : pm_LogFiles)
            {
                if (lv_LogFile.second.is_open())
                {
                    lv_LogFile.second.flush();
                }
            }

            pm_LogSizeCounter = 0; //reset since all logs have been flushed
        }

        static constexpr uint8_t
            ExtractLevelMask(uint32_t fp_Flags) noexcept
        {
            return static_cast<uint8_t>(fp_Flags & 0xFF);
        }

        static constexpr uint8_t
            ExtractFlushMask(uint32_t fp_Flags) noexcept
        {
            return static_cast<uint8_t>((fp_Flags >> 8) & 0xFF);
        }
    };
} //gonna implement namespace after i compile ig ill add that to the TODO -- when the fuck did i write this lmfao

//=========================================================================================== BEGINNING OF ACTUAL API ===========================================================================================//

namespace GGPO
{
#if defined(_WIN32) || defined(_WIN64)

    using GGPO_SOCKET = SOCKET;

    #define GGPO_INVALID_SOCKET (SOCKET)(~0)

    #define GGPO_GET_LAST_ERROR() WSAGetLastError()
    #define GGPO_NETWORK_ERROR_CODE DWORD

    #define GGPO_CLOSE_SOCKET(__arg) closesocket(__arg)
    #define GGPO_SOCKET_ERROR_CODE WSAEWOULDBLOCK

#else

    using GGPO_SOCKET = uint64_t;

    #define GGPO_INVALID_SOCKET (-1)

    #define GGPO_GET_LAST_ERROR() errno
    #define GGPO_NETWORK_ERROR_CODE uint32_t

    #define GGPO_CLOSE_SOCKET(__arg) close(__arg)
    #define GGPO_SOCKET_ERROR_CODE EWOULDBLOCK

#endif

    constexpr auto GGPO_SOCKET_ERROR = -1;
    constexpr auto MAX_UDP_ENDPOINTS = 16;

    constexpr int MAX_UDP_PACKET_SIZE = 4096;

    inline GGPO_SOCKET
        CreateSocket
        (
            uint16_t bind_port, 
            int retries,
            Logger* logger
        )
    {
        GGPO_SOCKET f_Socket;
        sockaddr_in f_SocketIn;
        uint16_t port;
        int optval = 1;

        f_Socket = socket(AF_INET, SOCK_DGRAM, 0);
        setsockopt(f_Socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof optval);
        //NOT SURE ABOUT DEFAULT SOCKET CONFIGS ON POSIX SYSTEMS VS MICROSOFT
        // setsockopt(f_Socket, SOL_SOCKET, SO_DONTLINGER, (const char*)&optval, sizeof optval);

        // non-blocking...
        #if defined(_WIN32) || defined(_WIN64)
            u_long iMode = 1;
            ioctlsocket(f_Socket, FIONBIO, &iMode);
        #else
            int flags = fcntl(f_Socket, F_GETFL, 0);
            fcntl(f_Socket, F_SETFL, flags | O_NONBLOCK);
        #endif

        f_SocketIn.sin_family = AF_INET;
        f_SocketIn.sin_addr.s_addr = htonl(INADDR_ANY);

        for (port = bind_port; port <= bind_port + retries; port++)
        {
            f_SocketIn.sin_port = htons(port);

            if (bind(f_Socket, (sockaddr*)&f_SocketIn, sizeof f_SocketIn) != GGPO_SOCKET_ERROR)
            {
                logger->Info(format("Udp bound to port: {}.", port), "udp.cpp");
                return f_Socket;
            }
        }

        GGPO_CLOSE_SOCKET(f_Socket);

        return GGPO_INVALID_SOCKET;
    }
}

//=========================================================================================== Platform Abstraction Tools ===========================================================================================//

namespace GGPO::Platform{

    using ProcessID =
    #if defined(_WIN32) || defined(_WIN64)
        DWORD;
    #else
        pid_t;
    #endif

    inline ProcessID 
        GetProcessID() 
    {
    #if defined(_WIN32) || defined(_WIN64)
        return GetCurrentProcessId();
    #else
        return getpid();
    #endif
    }

    inline void 
        AssertFailed
        (
            const char* fp_FileName, 
            int fp_LineNumber, 
            const char* fp_FailedExpr,
            ProcessID fp_ProcessID
        ) 
    {
        ostringstream f_AssertMsg;

        f_AssertMsg 
            << "Assertion failed: " << fp_FailedExpr << "\n"
            << "File: " << fp_FileName << "\n"
            << "Line: " << fp_LineNumber << "\n"
            << "Process ID: " << fp_ProcessID << "\n";

        string message = f_AssertMsg.str();

#if defined(_WIN32) || defined(_WIN64)
        MessageBoxA(nullptr, message.c_str(), "GGPO Assertion Failed", MB_OK | MB_ICONEXCLAMATION);
#else
        cerr << message << endl;
        // Optional: trigger debugger
        raise(SIGTRAP); // or: __builtin_trap();
#endif

        exit(EXIT_FAILURE);
    }


    // For game timers, netcode, frame delta, etc
inline uint64_t 
    GetMonotonicTimeMS() 
    noexcept
{
#if defined(_WIN32) || defined(_WIN64)
    return timeGetTime(); // 32-bit, wraps after ~49.7 days
#elif defined(__linux__) || defined(__APPLE__)
    struct timespec t{};
    clock_gettime(CLOCK_MONOTONIC, &t);
    return static_cast<uint64_t>(t.tv_sec) * 1000ULL + t.tv_nsec / 1000000ULL;
#else
    static_assert(false, "No monotonic time source available on this platform.");
#endif
}

// Other stuff im not sure about
#if defined(_WIN32) || defined(_WIN64)

inline optional<string>
    GetEnvVar(const char* name)
{
    char buf[1024];

    DWORD len = GetEnvironmentVariable(name, buf, sizeof(buf));

    if (len == 0 || len >= sizeof(buf))
    {
        PrintError("BUFFER OVERFLOW WHEN TRYING TO OBTAIN ENVIRONMENT VARIABLE ON WINDOWS UWU");
        return nullopt;
    }

    return string(buf, len);
}

inline int
    GetConfigInt(const char* name)
{
    auto val = GetEnvVar(name);
    return val ? atoi(val->c_str()) : 0;
}

inline bool
    GetConfigBool(const char* name)
{
    auto val = GetEnvVar(name);

    if (not val) 
    {
        return false;
    }

    return atoi(val->c_str()) != 0 || _stricmp(val->c_str(), "true") == 0;
}
    
#endif //Posix OS Check //Windows OS Check
}

//==================================================================================== GameInput ============================================================================================//

namespace GGPO{

    template<typename T, size_t pm_MaxCapacity>
    struct FixedPushBuffer
    {
        constexpr void PushBack(const T& fp_Item)
        {
            GGPO_ASSERT(pm_CurrentIndex < pm_MaxCapacity and "FixedPushBuffer overflowed!");
            pm_Data[pm_CurrentIndex++] = fp_Item;
        }

        template<typename... Args>
        constexpr void EmplaceBack(Args&&... fp_Args)
        {
            GGPO_ASSERT(pm_CurrentIndex < pm_MaxCapacity and "FixedPushBuffer overflowed!");
            pm_Data[pm_CurrentIndex++] = T(std::forward<Args>(fp_Args)...); // Construct T using args, assign to slot
        }

        [[nodiscard]] constexpr T& operator[](size_t fp_Index)
        {
            GGPO_ASSERT(fp_Index < pm_CurrentIndex);
            return pm_Data[fp_Index];
        }

        [[nodiscard]] constexpr const T& operator[](size_t fp_Index) const
        {
            GGPO_ASSERT(fp_Index < pm_CurrentIndex);
            return pm_Data[fp_Index];
        }

        [[nodiscard]] constexpr size_t CurrentSize() const noexcept
        {
            return pm_CurrentIndex;
        }

        constexpr void Clear() noexcept
        {
            pm_CurrentIndex = 0;
        }

    private:
        array<T, pm_MaxCapacity> pm_Data{};
        size_t pm_CurrentIndex = 0;
    };

}//namespace GGPO

//==================================================================================== GameInput ============================================================================================//

//==================================================================================== InputQueue ============================================================================================//

//==================================================================================== TimeSync ============================================================================================//

constexpr int FRAME_WINDOW_SIZE = 40;
constexpr int MIN_UNIQUE_FRAMES = 10;
constexpr int MIN_FRAME_ADVANTAGE = 3;
constexpr int MAX_FRAME_ADVANTAGE = 9;

 //==================================================================================== Sync ============================================================================================//

//==================================================================================== IPollSink ============================================================================================//

#if defined(_WIN32) || defined(_WIN64) //idfk
    #define GGPO_HANDLE HANDLE
#else
#define GGPO_HANDLE uint32_t
#endif

    constexpr int MAX_POLLABLE_HANDLES = 64;

 //==================================================================================== Udp ============================================================================================//

//==================================================================================== UdpMsg ============================================================================================//

constexpr int MAX_COMPRESSED_BITS = 4096;
constexpr int UDP_MSG_MAX_PLAYERS = 4;

//#pragma pack(push, 1)

struct UdpMsg
{
    enum MsgType
    {
        Invalid = 0,
        SyncRequest = 1,
        SyncReply = 2,
        Input = 3,
        QualityReport = 4,
        QualityReply = 5,
        KeepAlive = 6,
        InputAck = 7,
    };

    struct connect_status
    {
        bool disconnected; //= false; THESE initialized values cause a deleted function error ill change this later i want deafult initialized values for memory safety
        int last_frame; //= -1;
    };

    struct
    {
        uint16_t         magic;
        uint16_t         sequence_number;
        uint8_t          type;            /* packet type */
    } hdr;
    union
    {
        struct
        {
            uint32_t      random_request;  /* please reply back with this random data */
            uint16_t      remote_magic;
            uint8_t       remote_endpoint;
        } sync_request;

        struct
        {
            uint32_t      random_reply;    /* OK, here's your random data back */
        } sync_reply;

        struct
        {
            int8_t        frame_advantage; /* what's the other guy's frame advantage? */
            uint32_t      ping;
        } quality_report;

        struct
        {
            uint32_t      pong;
        } quality_reply;

        struct
        {
            connect_status    peer_connect_status[UDP_MSG_MAX_PLAYERS];

            uint32_t            start_frame;

            int               disconnect_requested : 1;
            int               ack_frame : 31;

            uint16_t            num_bits;
            uint8_t             input_size; // XXX: shouldn't be in every single packet!
            uint8_t             bits[MAX_COMPRESSED_BITS]; /* must be last */
        } input;

        struct
        {
            int               ack_frame : 31;
        } input_ack;

    } u;

public:
    int PacketSize()
    {
        return sizeof(hdr) + PayloadSize();
    }

    int PayloadSize()
    {
        int size;

        switch (hdr.type)
        {
        case SyncRequest:   return sizeof(u.sync_request);
        case SyncReply:     return sizeof(u.sync_reply);
        case QualityReport: return sizeof(u.quality_report);
        case QualityReply:  return sizeof(u.quality_reply);
        case InputAck:      return sizeof(u.input_ack);
        case KeepAlive:     return 0;
        case Input:
            size = (int)((char*)&u.input.bits - (char*)&u.input);
            size += (u.input.num_bits + 7) / 8;
            return size;
        }

        GGPO_ASSERT(false); // ??????????

        return 0;
    }

    UdpMsg(MsgType t) { hdr.type = (uint8_t)t; }
};

//#pragma pack(pop) 
//==================================================================================== UdpProtocol ============================================================================================//

constexpr int UDP_HEADER_SIZE = 28;     /* Size of IP + UDP headers */
constexpr int NUM_SYNC_PACKETS = 5;
constexpr int SYNC_RETRY_INTERVAL = 2000;
constexpr int SYNC_FIRST_RETRY_INTERVAL = 500;
constexpr int RUNNING_RETRY_INTERVAL = 200;
constexpr int KEEP_ALIVE_INTERVAL = 200;
constexpr int QUALITY_REPORT_INTERVAL = 1000;
constexpr int NETWORK_STATS_INTERVAL = 1000;
constexpr int UDP_SHUTDOWN_TIMER = 5000;
constexpr int MAX_SEQ_DISTANCE = (1 << 15);

//==================================================================================== SyncTest Backend ============================================================================================//


//==================================================================================== Spectator Backend ============================================================================================//

constexpr int SPECTATOR_FRAME_BUFFER_SIZE = 64;


//==================================================================================== P2P Backend ============================================================================================//

static constexpr int RECOMMENDATION_INTERVAL = 240;
static constexpr int DEFAULT_DISCONNECT_TIMEOUT = 5000;
static constexpr int DEFAULT_DISCONNECT_NOTIFY_START = 750;

 //=========================================================================================== Main API UwU ===========================================================================================//

namespace GGPO
{
    enum class SessionType
    {
        P2P,
        Spectator
    };

    struct Session
    {
        Session() = default;
        ~Session() = default;

        int 
            PollEvents(Event* fp_Out)
        {
            if (pm_EventQueue.IsEmpty()) //return false when passed a nullptr ref or queue is empty uwu
            {
                return 0; //business as usual
            }
            if (not fp_Out) //THROW ERROR: nullptr found when object was expected
            {
                logger->Error(format("Tried to pass a nullptr reference to Event inside PollEvents() within Session ID: {}", 69), "Session");
                return 0;
            }

            *fp_Out = pm_EventQueue.Front();
            pm_EventQueue.Pop();

            return 1;
        }

        ErrorCode 
            StartSession(SessionType mode) 
        {

            return ErrorCode::OK;
        }

        ErrorCode
            AddPlayer
            (
                Player* player,
                PlayerHandle* handle
            )
        {

            return ErrorCode::OK;
        }

        ErrorCode
            AddLocalInput
            (
                PlayerHandle player,
                void* values,
                int size
            )
        {
            if (not values)
            {
                Print("tried to pass nullptr reference to SyncInput() as the second argument");
                return ErrorCode::NULLPTR_PASSED_AS_VALUE;
            }

            return ErrorCode::OK;
        }

        ErrorCode
            SyncInput
            (
                void* values,
                int size,
                int* disconnect_flags
            )
        {
            if (not values)
            {
                Print("tried to pass nullptr reference to SyncInput() as the first argument");
                return ErrorCode::NULLPTR_PASSED_AS_VALUE;
            }
            if (not disconnect_flags)
            {
                Print("tried to pass nullptr reference to SyncInput() as the disconnect flags");
                return ErrorCode::NULLPTR_PASSED_AS_VALUE;
            }

            return ErrorCode::OK;
        }

        ErrorCode
            RunSyncTestChecks()
        {
            

            return ErrorCode::OK;
        }

        ErrorCode
            DoPoll(int timeout)
        {

            return ErrorCode::OK;
        }

        ErrorCode
            IncrementFrame(void)
        {

            return ErrorCode::OK;
        }

        ErrorCode
            Chat(char* text)
        {

            return ErrorCode::OK;
        }

        ErrorCode
            DisconnectPlayer(PlayerHandle handle)
        {

            return ErrorCode::OK;
        }

        ErrorCode
            GetNetworkStats
            (
                NetworkStats* stats,
                PlayerHandle handle
            )
        {

            return ErrorCode::OK;
        }

        ErrorCode
            SetFrameDelay
            (
                PlayerHandle player,
                int delay
            )
        {

            return ErrorCode::OK;
        }

        ErrorCode
            SetDisconnectTimeout(int timeout)
        {

            return ErrorCode::OK;
        }

        ErrorCode
            SetDisconnectNotifyStart(int timeout)
        {

            return ErrorCode::OK;
        }


    protected:
        bool pm_IsHost = false;
        bool pm_IsInRollback = false;

        unique_ptr<Logger> logger = nullptr;
        RingBuffer<Event, 64> pm_EventQueue;

        // Sync pm_Sync;
        // Udp pm_Udp;

        // vector<UdpProtocol> pm_Endpoints; //needs to be dynamically sized ig for now since total size of lobby isn't knowable from compile time

        // unique_ptr<SyncTestBackend> pm_SyncTestBackend = nullptr;

        // unique_ptr<Peer2PeerBackend> pm_P2P = nullptr;
        // vector<SpectatorBackend> pm_Spectators;
    };
}

 //=========================================================================================== END OwO ===========================================================================================//