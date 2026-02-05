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

#ifndef GGPO_MAX_INT
#  define GGPO_MAX_INT   18446744073709551615 //largest value for a unsigned 64 bit int
#endif

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


#define GGPO_ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])

#define GGPO_DEFAULT_LOGGER_FLAGS Logger::Flags::ALL_LOGS | Logger::Flags::FLUSH_ERROR | Logger::Flags::FLUSH_FATAL

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
        Spectator,
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
            EmplaceOverwrite(forward<Args>(fp_Args)...);
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
            pm_Data[pm_CurrentIndex++] = T(forward<Args>(args)...); // Construct T using args, assign to slot
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

            return move(f_CreatedLogger);
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

                    pm_LogFiles[fp_FileName] = move(f_LogFile);

                    ////////////////////////////////////////////// Success! //////////////////////////////////////////////

                    return true;
                }
            }

            ////////////////////////////////////////////// If file doesn't exist or isn't too big it's business as usual UwU //////////////////////////////////////////////

            ofstream f_LogFile(f_FullPath, ios::out | ios::app);

            if (not f_LogFile.is_open())
            {
                PrintError(format("Failed to open log file: '{}' with logger named: {}", fp_FileName, pm_LoggerName));
                return false;
            }

            pm_LogFiles[fp_FileName] = move(f_LogFile);

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

//=========================================================================================== BitVector Stuff ===========================================================================================//

namespace GGPO
{
    constexpr const int GGPO_BITVECTOR_NIBBLE_SIZE = 8;

    void
        BitVector_SetBit(uint8_t* vector, int* offset)
    {
        vector[(*offset) / 8] |= (1 << ((*offset) % 8));
        *offset += 1;
    }

    void
        BitVector_ClearBit(uint8_t* vector, int* offset)
    {
        vector[(*offset) / 8] &= ~(1 << ((*offset) % 8));
        *offset += 1;
    }

    void
        BitVector_WriteNibblet(uint8_t* vector, int nibble, int* offset)
    {
        GGPO_ASSERT(nibble < (1 << GGPO_BITVECTOR_NIBBLE_SIZE));

        for (int i = 0; i < GGPO_BITVECTOR_NIBBLE_SIZE; i++)
        {
            if (nibble & (1 << i))
            {
                BitVector_SetBit(vector, offset);
            }
            else
            {
                BitVector_ClearBit(vector, offset);
            }
        }
    }

    int
        BitVector_ReadBit(uint8_t* vector, int* offset)
    {
        int retval = !!(vector[(*offset) / 8] & (1 << ((*offset) % 8)));
        *offset += 1;
        return retval;
    }

    int
        BitVector_ReadNibblet(uint8_t* vector, int* offset)
    {
        int nibblet = 0;

        for (int i = 0; i < GGPO_BITVECTOR_NIBBLE_SIZE; i++)
        {
            nibblet |= (BitVector_ReadBit(vector, offset) << i);
        }

        return nibblet;
    }
}

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

// GAMEINPUT_MAX_BYTES * GAMEINPUT_MAX_PLAYERS * 8 must be less than
// 2^BITVECTOR_NIBBLE_SIZE (see ln 1189)

constexpr int GAMEINPUT_MAX_BYTES = 9;
constexpr int GAMEINPUT_MAX_PLAYERS = 2;

namespace GGPO
{
    struct GameInput
    {
        enum Constants
        {
            NullFrame = -1
        };

        int      frame;
        int      size; /* size in bytes of the entire input for all players */
        char     bits[GAMEINPUT_MAX_BYTES * GAMEINPUT_MAX_PLAYERS];

        bool
            is_null()
            const
        {
            return frame == NullFrame;
        }

        void
            erase()
        {
            memset(bits, 0, sizeof(bits));
        }

        void 
            init
            (
                int iframe,
                char* ibits,
                int isize,
                int offset
            )
        {
            GGPO_ASSERT(isize);
            GGPO_ASSERT(isize <= GAMEINPUT_MAX_BYTES);
            frame = iframe;
            size = isize;
            memset(bits, 0, sizeof(bits));

            if (ibits)
            {
                memcpy(bits + (offset * isize), ibits, isize);
            }
        }

        void 
            init
            (
                int iframe, 
                char* ibits, 
                int isize
            )
        {
            GGPO_ASSERT(isize);
            GGPO_ASSERT(isize <= GAMEINPUT_MAX_BYTES * GAMEINPUT_MAX_PLAYERS);

            frame = iframe;
            size = isize;
            memset(bits, 0, sizeof(bits));

            if (ibits)
            {
                memcpy(bits, ibits, isize);
            }
        }
        bool 
            value(int i) 
            const 
        { 
            return (bits[i / 8] & (1 << (i % 8))) != 0; 
        }

        void 
            set(int i) 
        { 
            bits[i / 8] |= (1 << (i % 8)); 
        }

        void 
            clear(int i) 
        { 
            bits[i / 8] &= ~(1 << (i % 8)); 
        }

        string 
            Description
            (
                bool show_frame = true
            ) 
            const
        {
            GGPO_ASSERT(size);
            string f_Description;

            if (show_frame)
            {
                f_Description = format("(frame: {} size: {}", frame, size);
            }
            else
            {
                f_Description = format("(size: {} ", size);
            }

            return f_Description;
        }

        bool 
            equal
            (
                GameInput& other,
                bool bitsonly = false,
                Logger* logger
            )
        {
            if (not bitsonly and frame != other.frame)
            {
                logger->Info(format("frames don't match: {}, {}", frame, other.frame), "game_input.cpp");
            }
            if (size != other.size)
            {
                logger->Info(format("sizes don't match: {}, {}", size, other.size), "game_input.cpp");
            }
            if (memcmp(bits, other.bits, size))
            {
                logger->Info("bits don't match", "game_input.cpp");
            }

            GGPO_ASSERT(size and other.size);

            return
                (bitsonly or frame == other.frame) and
                size == other.size and
                memcmp(bits, other.bits, size) == 0
                ;
        }
    };
}

//==================================================================================== InputQueue ============================================================================================//

#define PREVIOUS_FRAME(offset)   (((offset) == 0) ? (INPUT_QUEUE_LENGTH - 1) : ((offset) - 1))

namespace GGPO
{
    constexpr int INPUT_QUEUE_LENGTH = 128;
    constexpr int DEFAULT_INPUT_SIZE = 4;

    class InputQueue
    {
    private:
        unique_ptr<Logger> input_queue_logger = nullptr;

    public:
        InputQueue(int input_size = DEFAULT_INPUT_SIZE) //???? why do it like this lmfao y not just do it inside the constructor body lol
        {
           input_queue_logger = Logger::CreateUnique("InputQueueLogger", GGPO_DEFAULT_LOGGER_FLAGS, GGPO_DEFAULT_LOG_OUTPUT_DIRECTORY);

           GGPO_ASSERT(input_queue_logger)
            
            Init(-1, input_size);
        };

        ~InputQueue() = default;

    public:
        void
            Init(int id, int input_size)
        {
            _id = id;
            _head = 0;
            _tail = 0;
            _length = 0;
            _frame_delay = 0;
            _first_frame = true;
            _last_user_added_frame = GameInput::NullFrame;
            _first_incorrect_frame = GameInput::NullFrame;
            _last_frame_requested = GameInput::NullFrame;
            _last_added_frame = GameInput::NullFrame;

            _prediction.init(GameInput::NullFrame, NULL, input_size);

            /*
             * This is safe because we know the GameInput is a proper structure (as in,
             * no virtual methods, no contained classes, etc.).
             */
            memset(_inputs, 0, sizeof _inputs);

            for (int i = 0; i < GGPO_ARRAY_SIZE(_inputs); i++)
            {
                _inputs[i].size = input_size;
            }
        }

        int
            GetLastConfirmedFrame()
        {
            input_queue_logger->Info(format("returning last confirmed frame {}.", _last_added_frame), "input_queue.cpp");
            return _last_added_frame;
        }

        int
            GetFirstIncorrectFrame()
            const
        {
            return _first_incorrect_frame;
        }

        int
            GetLength()
            const
        {
            return _length;
        }

        void 
            SetFrameDelay(int delay) 
        {
            _frame_delay = delay; 
        }

        void
            ResetPrediction
        (
            int frame
         )
        {
            GGPO_ASSERT(_first_incorrect_frame == GameInput::NullFrame || frame <= _first_incorrect_frame);

            input_queue_logger->Info(format("resetting all prediction errors back to frame {}.", frame), "input_queue.cpp");

            /*
             * There's nothing really to do other than reset our prediction
             * state and the incorrect frame counter...
             */
            _prediction.frame = GameInput::NullFrame;
            _first_incorrect_frame = GameInput::NullFrame;
            _last_frame_requested = GameInput::NullFrame;
        }

        void
            DiscardConfirmedFrames
        (
            int frame
        )
        {
            GGPO_ASSERT(frame >= 0);

            if (_last_frame_requested != GameInput::NullFrame)
            {
                frame = GGPO_MIN(frame, _last_frame_requested);
            }

            input_queue_logger->Info(format("discarding confirmed frames up to {} (last_added:{} length:{} [head:{} tail:{}]).", frame, _last_added_frame, _length, _head, _tail), "input_queue.cpp");

            if (frame >= _last_added_frame)
            {
                _tail = _head;
            }
            else
            {
                int offset = frame - _inputs[_tail].frame + 1;

                input_queue_logger->Info(format("difference of {} frames.", offset), "input_queue.cpp");
                GGPO_ASSERT(offset >= 0);

                _tail = (_tail + offset) % INPUT_QUEUE_LENGTH;
                _length -= offset;
            }

            input_queue_logger->Info(format("after discarding, new tail is {} (frame:{}).", _tail, _inputs[_tail].frame), "input_queue.cpp");
            GGPO_ASSERT(_length >= 0);
        }

        bool
            GetConfirmedInput
            (
                int requested_frame, 
                GameInput* input
            )
            const
        {
            GGPO_ASSERT(_first_incorrect_frame == GameInput::NullFrame || requested_frame < _first_incorrect_frame);
            int offset = requested_frame % INPUT_QUEUE_LENGTH;

            if (_inputs[offset].frame != requested_frame)
            {
                return false;
            }

            *input = _inputs[offset];

            return true;
        }

        bool
            GetInput
            (
                int requested_frame, 
                GameInput* input
            )
        {
            input_queue_logger->Info(format("requesting input frame {}.", requested_frame), "input_queue.cpp");

            /*
            * No one should ever try to grab any input when we have a prediction
            * error.  Doing so means that we're just going further down the wrong
            * path.  GGPO_ASSERT this to verify that it's true.
            */
            GGPO_ASSERT(_first_incorrect_frame == GameInput::NullFrame);

            /*
            * Remember the last requested frame number for later.  We'll need
            * this in AddInput() to drop out of prediction mode.
            */
            _last_frame_requested = requested_frame;

            GGPO_ASSERT(requested_frame >= _inputs[_tail].frame);

            if (_prediction.frame == GameInput::NullFrame)
            {
                /*
                * If the frame requested is in our range, fetch it out of the queue and
                * return it.
                */
                int offset = requested_frame - _inputs[_tail].frame;

                if (offset < _length)
                {
                    offset = (offset + _tail) % INPUT_QUEUE_LENGTH;
                    GGPO_ASSERT(_inputs[offset].frame == requested_frame);
                    *input = _inputs[offset];
                    input_queue_logger->Info(format("returning confirmed frame number {}.", input->frame), "input_queue.cpp");
                    return true;
                }

                /*
                * The requested frame isn't in the queue.  Bummer.  This means we need
                * to return a prediction frame.  Predict that the user will do the
                * same thing they did last time.
                */
                if (requested_frame == 0)
                {
                    input_queue_logger->Info("basing new prediction frame from nothing, you're client wants frame 0.", "input_queue.cpp");
                    _prediction.erase();
                }
                else if (_last_added_frame == GameInput::NullFrame)
                {
                    input_queue_logger->Info("basing new prediction frame from nothing, since we have no frames yet.", "input_queue.cpp");
                    _prediction.erase();
                }
                else
                {
                    input_queue_logger->Info(format("basing new prediction frame from previously added frame (queue entry:{}, frame:{}).", PREVIOUS_FRAME(_head), _inputs[PREVIOUS_FRAME(_head)].frame), "input_queue.cpp");
                    _prediction = _inputs[PREVIOUS_FRAME(_head)];
                }
                _prediction.frame++;
            }

            GGPO_ASSERT(_prediction.frame >= 0);

            /*
            * If we've made it this far, we must be predicting.  Go ahead and
            * forward the prediction frame contents.  Be sure to return the
            * frame number requested by the client, though.
            */
            *input = _prediction;
            input->frame = requested_frame;
            input_queue_logger->Info(format("returning prediction frame number {} ({}).", input->frame, _prediction.frame), "input_queue.cpp");

            return false;
        }

        void
            AddInput(GameInput& input)
        {
            int new_frame;

            input_queue_logger->Info(format("adding input frame number {} to queue.", input.frame), "input_queue.cpp");

            /*
            * These next two lines simply verify that inputs are passed in
            * sequentially by the user, regardless of frame delay.
            */
            GGPO_ASSERT(_last_user_added_frame == GameInput::NullFrame || input.frame == _last_user_added_frame + 1);

            _last_user_added_frame = input.frame;

            /*
            * Move the queue head to the correct point in preparation to
            * input the frame into the queue.
            */
            new_frame = AdvanceQueueHead(input.frame);

            if (new_frame != GameInput::NullFrame)
            {
                AddDelayedInputToQueue(input, new_frame);
            }

            /*
            * Update the frame number for the input.  This will also set the
            * frame to GameInput::NullFrame for frames that get dropped (by
            * design).
            */
            input.frame = new_frame;
        }

    protected:

        int
            AdvanceQueueHead(int frame)
        {
            input_queue_logger->Info(format("advancing queue head to frame {}.", frame), "input_queue.cpp");

            int expected_frame = _first_frame ? 0 : _inputs[PREVIOUS_FRAME(_head)].frame + 1;

            frame += _frame_delay;

            if (expected_frame > frame)
            {
                /*
                * This can occur when the frame delay has dropped since the last
                * time we shoved a frame into the system.  In this case, there's
                * no room on the queue.  Toss it.
                */
                input_queue_logger->Info(format("Dropping input frame {} (expected next frame to be {}).", frame, expected_frame), "input_queue.cpp");
                return GameInput::NullFrame;
            }

            while (expected_frame < frame)
            {
                /*
                * This can occur when the frame delay has been increased since the last
                * time we shoved a frame into the system.  We need to replicate the
                * last frame in the queue several times in order to fill the space
                * left.
                */
                input_queue_logger->Info(format("Adding padding frame {} to account for change in frame delay.", expected_frame), "input_queue.cpp");
                GameInput& last_frame = _inputs[PREVIOUS_FRAME(_head)];
                AddDelayedInputToQueue(last_frame, expected_frame);
                expected_frame++;
            }

            GGPO_ASSERT(frame == 0 || frame == _inputs[PREVIOUS_FRAME(_head)].frame + 1);
            return frame;
        }

        void
            AddDelayedInputToQueue(GameInput& input, int frame_number)
        {
            input_queue_logger->Info(format("adding delayed input frame number {} to queue.", frame_number), "input_queue.cpp");

            GGPO_ASSERT(input.size == _prediction.size);

            GGPO_ASSERT(_last_added_frame == GameInput::NullFrame || frame_number == _last_added_frame + 1);

            GGPO_ASSERT(frame_number == 0 || _inputs[PREVIOUS_FRAME(_head)].frame == frame_number - 1);

            /*
            * Add the frame to the back of the queue
            */
            _inputs[_head] = input;
            _inputs[_head].frame = frame_number;
            _head = (_head + 1) % INPUT_QUEUE_LENGTH;
            _length++;
            _first_frame = false;

            _last_added_frame = frame_number;

            if (_prediction.frame != GameInput::NullFrame)
            {
                GGPO_ASSERT(frame_number == _prediction.frame);

                /*
                * We've been predicting...  See if the inputs we've gotten match
                * what we've been predicting.  If so, don't worry about it.  If not,
                * remember the first input which was incorrect so we can report it
                * in GetFirstIncorrectFrame()
                */
                if (_first_incorrect_frame == GameInput::NullFrame and not _prediction.equal(input, true))
                {
                    input_queue_logger->Info(format("frame {} does not match prediction.  marking error.", frame_number), "input_queue.cpp");
                    _first_incorrect_frame = frame_number;
                }

                /*
                * If this input is the same frame as the last one requested and we
                * still haven't found any mis-predicted inputs, we can dump out
                * of predition mode entirely!  Otherwise, advance the prediction frame
                * count up.
                */
                if (_prediction.frame == _last_frame_requested and _first_incorrect_frame == GameInput::NullFrame)
                {
                    input_queue_logger->Info("prediction is correct!  dumping out of prediction mode.", "input_queue.cpp");
                    _prediction.frame = GameInput::NullFrame;
                }
                else
                {
                    _prediction.frame++;
                }
            }
            GGPO_ASSERT(_length <= INPUT_QUEUE_LENGTH);
        }

    protected:
        int                  _id;
        int                  _head;
        int                  _tail;
        int                  _length;
        bool                 _first_frame;

        int                  _last_user_added_frame;
        int                  _last_added_frame;
        int                  _first_incorrect_frame;
        int                  _last_frame_requested;

        int                  _frame_delay;

        GameInput            _inputs[INPUT_QUEUE_LENGTH];
        GameInput            _prediction;
    };
}

//==================================================================================== TimeSync ============================================================================================//

constexpr int FRAME_WINDOW_SIZE = 40;
constexpr int MIN_UNIQUE_FRAMES = 10;
constexpr int MIN_FRAME_ADVANTAGE = 3;
constexpr int MAX_FRAME_ADVANTAGE = 9;

 namespace GGPO
 { 
      class TimeSync
      {
      public:
          TimeSync()
          {
              memset(_local, 0, sizeof(_local));
              memset(_remote, 0, sizeof(_remote));
              _next_prediction = FRAME_WINDOW_SIZE * 3;
          }

          virtual ~TimeSync() = default;
 
          void
              advance_frame
              (
                  GameInput& input,
                  int advantage,
                  int radvantage
              )
          {
              // Remember the last frame and frame advantage
              _last_inputs[input.frame % GGPO_ARRAY_SIZE(_last_inputs)] = input;
              _local[input.frame % GGPO_ARRAY_SIZE(_local)] = advantage;
              _remote[input.frame % GGPO_ARRAY_SIZE(_remote)] = radvantage;
          }

          int
              recommend_frame_wait_duration
              (
                bool require_idle_input,
                Logger* logger
              )
          {
              // Average our local and remote frame advantages
              int i, sum = 0;
              float advantage, radvantage;

              for (i = 0; i < GGPO_ARRAY_SIZE(_local); i++)
              {
                  sum += _local[i];
              }

              advantage = sum / (float)GGPO_ARRAY_SIZE(_local);
              sum = 0;

              for (i = 0; i < GGPO_ARRAY_SIZE(_remote); i++)
              {
                  sum += _remote[i];
              }

              radvantage = sum / (float)GGPO_ARRAY_SIZE(_remote);

              static atomic<int> count = 0;
              count++;

              // See if someone should take action.  The person furthest ahead
              // needs to slow down so the other user can catch up.
              // Only do this if both clients agree on who's ahead!!
              if (advantage >= radvantage)
              {
                  return 0;
              }

              // Both clients agree that we're the one ahead.  Split
              // the difference between the two to figure out how long to
              // sleep for.
              int sleep_frames = (int)(((radvantage - advantage) / 2) + 0.5);

              logger->Info(format("iteration {}:  sleep frames is {}", count, sleep_frames), "timesync.cpp");

              // Some things just aren't worth correcting for.  Make sure
              // the difference is relevant before proceeding.
              if (sleep_frames < MIN_FRAME_ADVANTAGE)
              {
                  return 0;
              }

              // Make sure our input had been "idle enough" before recommending
              // a sleep.  This tries to make the emulator sleep while the
              // user's input isn't sweeping in arcs (e.g. fireball motions in
              // Street Fighter), which could cause the player to miss moves.
              if (require_idle_input)
              {
                  for (i = 1; i < GGPO_ARRAY_SIZE(_last_inputs); i++)
                  {
                      if (not _last_inputs[i].equal(_last_inputs[0], true))
                      {
                          logger->Info(format("iteration {}:  rejecting due to input stuff at position {}...!!!", count, i), "timesync.cpp");
                          return 0;
                      }
                  }
              }

              // Success!!! Recommend the number of frames to sleep and adjust
              return GGPO_MIN(sleep_frames, MAX_FRAME_ADVANTAGE);
          }
 
      protected:
          int         _local[FRAME_WINDOW_SIZE];
          int         _remote[FRAME_WINDOW_SIZE];
          GameInput   _last_inputs[MIN_UNIQUE_FRAMES];
          int         _next_prediction;
      };
 }
 
 //==================================================================================== Sync ============================================================================================//

namespace GGPO
{
     class SyncTestBackend;

     class Sync
     {
    private:
        unique_ptr<Logger> sync_logger = nullptr;

     public:
         struct Config
         {
             int                     num_prediction_frames;
             int                     num_players;
             int                     input_size;
         };
         struct Event
         {
             enum
             {
                 ConfirmedInput,
             } type;
             union
             {
                 struct confirmedInput
                 {
                     GameInput   input;
                 };
             } u;
         };

     public:
         Sync(UdpMsg::connect_status* connect_status) :
             _local_connect_status(connect_status),
             _input_queues(NULL)
         {
             sync_logger = Logger::CreateUnique("SyncLogger", GGPO_DEFAULT_LOGGER_FLAGS, GGPO_DEFAULT_LOG_OUTPUT_DIRECTORY);

             GGPO_ASSERT(sync_logger)

             _framecount = 0;
             _last_confirmed_frame = -1;
             _max_prediction_frames = 0;
             memset(&_savedstate, 0, sizeof(_savedstate));
         }

         virtual ~Sync()
         {
             /*
              * Delete frames manually here rather than in a destructor of the SavedFrame
              * structure so we can efficently copy frames via weak references.
              */

             delete[] _input_queues;
             _input_queues = NULL;
         }

         void
             Init(Sync::Config& config)
         {
             _config = config;
             _framecount = 0;

             _max_prediction_frames = config.num_prediction_frames;

             CreateQueues(config);
         }

         void
             SetLastConfirmedFrame(int frame)
         {
             _last_confirmed_frame = frame;
             if (_last_confirmed_frame > 0)
             {
                 for (int i = 0; i < _config.num_players; i++)
                 {
                     _input_queues[i].DiscardConfirmedFrames(frame - 1);
                 }
             }
         }

         void
             SetFrameDelay(int queue, int delay)
         {
             _input_queues[queue].SetFrameDelay(delay);
         }

         bool
             AddLocalInput(int queue, GameInput& input)
         {
             int frames_behind = _framecount - _last_confirmed_frame;

             if (_framecount >= _max_prediction_frames && frames_behind >= _max_prediction_frames)
             {
                 sync_logger->Info("Rejecting input from emulator: reached prediction barrier.", "sync.cpp");
                 return false;
             }

             if (_framecount == 0)
             {
                 SaveCurrentFrame();
             }

             sync_logger->Info(format("Sending undelayed local frame {} to queue {}.", _framecount, queue), "sync.cpp");

             input.frame = _framecount;
             _input_queues[queue].AddInput(input);

             return true;
         }

         void
             AddRemoteInput(int queue, GameInput& input)
         {
             _input_queues[queue].AddInput(input);
         }

         int
             GetConfirmedInputs(void* values, int size, int frame)
         {
             int disconnect_flags = 0;
             char* output = (char*)values;

             GGPO_ASSERT(size >= _config.num_players * _config.input_size);

             memset(output, 0, size);

             for (int i = 0; i < _config.num_players; i++)
             {
                 GameInput input;
                 if (_local_connect_status[i].disconnected && frame > _local_connect_status[i].last_frame)
                 {
                     disconnect_flags |= (1 << i);
                     input.erase();
                 }
                 else
                 {
                     _input_queues[i].GetConfirmedInput(frame, &input);
                 }
                 memcpy(output + (i * _config.input_size), input.bits, _config.input_size);
             }
             return disconnect_flags;
         }

         int
             SynchronizeInputs(void* values, int size)
         {
             int disconnect_flags = 0;
             char* output = (char*)values;

             GGPO_ASSERT(size >= _config.num_players * _config.input_size);

             memset(output, 0, size);
             for (int i = 0; i < _config.num_players; i++)
             {
                 GameInput input;

                 if (_local_connect_status[i].disconnected and _framecount > _local_connect_status[i].last_frame)
                 {
                     disconnect_flags |= (1 << i);
                     input.erase();
                 }
                 else
                 {
                     _input_queues[i].GetInput(_framecount, &input);
                 }
                 memcpy(output + (i * _config.input_size), input.bits, _config.input_size);
             }
             return disconnect_flags;
         }

         void
             IncrementFrame(void)
         {
             _framecount++;
             SaveCurrentFrame();
         }

         int
             GetFrameCount()
             const
         {
             return _framecount;
         }

         bool
             GetEvent(Event& e)
         {
             if (_event_queue.CurrentSize())
             {
                 e = _event_queue.Front();
                 _event_queue.Pop();
                 return true;
             }

             return false;
         }

     protected:
         friend SyncTestBackend;

         struct SavedFrame
         {
             string buf;
             int      frame;
             int      checksum;
             SavedFrame() : buf(), frame(-1), checksum(0) { }
         };
         struct SavedState
         {
             SavedFrame frames[MAX_PREDICTION_FRAMES + 2];
             int head;
         };

         void
             LoadFrame(int frame)
         {
             //// find the frame in question
             //if (frame == _framecount)
             //{
             //    sync_logger->Info("Skipping NOP.", "sync.cpp");
             //    return;
             //}

             //// Move the head pointer back and load it up
             //_savedstate.head = FindSavedFrameIndex(frame);
             //SavedFrame* state = _savedstate.frames + _savedstate.head;

             //sync_logger->Info(format("=== Loading frame info {} (checksum: {}).", state->frame, state->checksum), "sync.cpp");

             //GGPO_ASSERT(state->buf and state->cbuf);

             //_callbacks.load_game_state(state->buf);

             //// Reset framecount and the head of the state ring-buffer to point in
             //// advance of the current frame (as if we had just finished executing it).
             //_framecount = state->frame;
             //_savedstate.head = (_savedstate.head + 1) % GGPO_ARRAY_SIZE(_savedstate.frames);
         }

         void
             SaveCurrentFrame()
         {
             ///*
             // * See StateCompress for the real save feature implemented by FinalBurn.
             // * Write everything into the head, then advance the head pointer.
             // */
             //SavedFrame* state = _savedstate.frames + _savedstate.head;

             //state->frame = _framecount;
             //_callbacks.save_game_state(state->buf, &state->frame, &state->checksum, state->frame);

             //sync_logger->Info(format("=== Saved frame info {} (checksum: {}).", state->frame, state->checksum), "sync.cpp");
             //_savedstate.head = (_savedstate.head + 1) % GGPO_ARRAY_SIZE(_savedstate.frames);
         }

         int
             FindSavedFrameIndex(int frame)
         {
             int i, count = GGPO_ARRAY_SIZE(_savedstate.frames);

             for (i = 0; i < count; i++)
             {
                 if (_savedstate.frames[i].frame == frame)
                 {
                     break;
                 }
             }
             if (i == count)
             {
                 GGPO_ASSERT(FALSE);
             }

             return i;
         }

         SavedFrame&
             GetLastSavedFrame()
         {
             int i = _savedstate.head - 1;

             if (i < 0)
             {
                 i = GGPO_ARRAY_SIZE(_savedstate.frames) - 1;
             }

             return _savedstate.frames[i];
         }

         bool
             CreateQueues(Config& config)
         {
             delete[] _input_queues;
             _input_queues = new InputQueue[_config.num_players];

             for (int i = 0; i < _config.num_players; i++)
             {
                 _input_queues[i].Init(i, _config.input_size);
             }

             return true;
         }

         void
             ResetPrediction(int frameNumber)
         {
             for (int i = 0; i < _config.num_players; i++)
             {
                 _input_queues[i].ResetPrediction(frameNumber);
             }
         }

     protected:
         SavedState     _savedstate;
         Config         _config;

         int            _last_confirmed_frame;
         int            _framecount;
         int            _max_prediction_frames;

         InputQueue* _input_queues;

         RingBuffer<Event, 32> _event_queue;
         UdpMsg::connect_status* _local_connect_status;
     };

}

//==================================================================================== IPollSink ============================================================================================//

#if defined(_WIN32) || defined(_WIN64) //idfk
    #define GGPO_HANDLE HANDLE
 #else
    #define GGPO_HANDLE uint32_t
 #endif

 namespace GGPO
 {
     constexpr int MAX_POLLABLE_HANDLES = 64;
 
      class IPollSink 
      {
      public:
          virtual ~IPollSink() { }
          virtual bool OnHandlePoll(void*) { return true; }
          virtual bool OnMsgPoll(void*) { return true; }
          virtual bool OnPeriodicPoll(void*, int) { return true; }
          virtual bool OnLoopPoll(void*) { return true; }
      };
 
      class Poll 
      {
      public:

        Poll(void) :
            _handle_count(0),
            _start_time(0)
        {
            /*
            * Create a dummy handle to simplify things.
            */
            //MSVC GARBO NEED A UNIX REPLACEMENT IF EVEN REQUIRED
            #if defined(_WIN32) || defined(_WIN64)
                _handles[_handle_count++] = CreateEvent(NULL, true, false, NULL);
            #else

            #endif
        }

          void
              RegisterHandle
              (
                  IPollSink* sink,
                  HANDLE h,
                  void* cookie = NULL
              )
          {
              GGPO_ASSERT(_handle_count < MAX_POLLABLE_HANDLES - 1);

              _handles[_handle_count] = h;
              _handle_sinks[_handle_count] = PollSinkCb(sink, cookie);
              _handle_count++;
          }

          void 
              RegisterMsgLoop
              (
                  IPollSink* sink, 
                  void* cookie = NULL
              ) 
          {
              _msg_sinks.PushBack(PollSinkCb(sink, cookie));
          }
          void 
              RegisterPeriodic
              (
                  IPollSink* sink, 
                  int interval, 
                  void* cookie = NULL
              ) 
          {
              _periodic_sinks.PushBack(PollPeriodicSinkCb(sink, cookie, interval));
          }

          void
              RegisterLoop
              (
                  IPollSink* sink, 
                  void* cookie = NULL
              ) 
          {
              _loop_sinks.PushBack(PollSinkCb(sink, cookie));
          }
 
          void
              Run()
          {
              while (Pump(100))
              {
                  continue; //? y continue lmfao i mean i get it but like ok ig
              }
          }

          bool
              Pump(int timeout)
          {
              int i, res;
              bool finished = false;

              if (_start_time == 0)
              {
                  _start_time = Platform::GetMonotonicTimeMS();
              }

              int elapsed = Platform::GetMonotonicTimeMS() - _start_time;
              int maxwait = ComputeWaitTime(elapsed);

              if (maxwait != INFINITE)
              {
                  timeout = GGPO_MIN(timeout, maxwait);
              }

              res = WaitForMultipleObjects(_handle_count, _handles, false, timeout);

              if (res >= WAIT_OBJECT_0 && res < WAIT_OBJECT_0 + _handle_count)
              {
                  i = res - WAIT_OBJECT_0;
                  finished = not _handle_sinks[i].sink->OnHandlePoll(_handle_sinks[i].cookie) or finished;
              }

              for (i = 0; i < _msg_sinks.CurrentSize(); i++)
              {
                  PollSinkCb& cb = _msg_sinks[i];
                  finished = not cb.sink->OnMsgPoll(cb.cookie) or finished;
              }

              for (i = 0; i < _periodic_sinks.CurrentSize(); i++)
              {
                  PollPeriodicSinkCb& cb = _periodic_sinks[i];

                  if (cb.interval + cb.last_fired <= elapsed)
                  {
                      cb.last_fired = (elapsed / cb.interval) * cb.interval;
                      finished = not cb.sink->OnPeriodicPoll(cb.cookie, cb.last_fired) or finished;
                  }
              }

              for (i = 0; i < _loop_sinks.CurrentSize(); i++)
              {
                  PollSinkCb& cb = _loop_sinks[i];
                  finished = not cb.sink->OnLoopPoll(cb.cookie) or finished;
              }

              return finished;
          }
 
        protected:
            int
                ComputeWaitTime(int elapsed)
          {
              int waitTime = INFINITE;
              size_t count = _periodic_sinks.CurrentSize();

              if (count > 0)
              {
                  for (int i = 0; i < count; i++)
                  {
                      PollPeriodicSinkCb& cb = _periodic_sinks[i];
                      int timeout = (cb.interval + cb.last_fired) - elapsed;

                      if (waitTime == INFINITE or (timeout < waitTime))
                      {
                          waitTime = GGPO_MAX(timeout, 0);
                      }
                  }
              }
              return waitTime;
          }
 
          struct PollSinkCb 
          {
              IPollSink* sink;
              void* cookie;
              PollSinkCb() : sink(NULL), cookie(NULL) { }
              PollSinkCb(IPollSink* s, void* c) : sink(s), cookie(c) { }
          };
 
          struct PollPeriodicSinkCb : public PollSinkCb 
          {
              int         interval;
              int         last_fired;
              PollPeriodicSinkCb() : PollSinkCb(NULL, NULL), interval(0), last_fired(0) { }
              PollPeriodicSinkCb(IPollSink* s, void* c, int i) :
                  PollSinkCb(s, c), interval(i), last_fired(0) { }
          };
 
          int               _start_time;
          int               _handle_count;
          HANDLE            _handles[MAX_POLLABLE_HANDLES];
          PollSinkCb        _handle_sinks[MAX_POLLABLE_HANDLES];
 
          FixedPushBuffer<PollSinkCb, 16>          _msg_sinks;
          FixedPushBuffer<PollSinkCb, 16>          _loop_sinks;
          FixedPushBuffer<PollPeriodicSinkCb, 16>  _periodic_sinks;
      };
 }

 //==================================================================================== Udp ============================================================================================//

namespace GGPO
{
     class Udp
     {
     public:
         struct Stats
         {
             int      bytes_sent;
             int      packets_sent;
             float    kbps_sent;
         };

         struct Callbacks
         {
             virtual ~Callbacks() { }
             virtual void OnMsg(sockaddr_in& from, UdpMsg* msg, int len) = 0;
         };

     public:
         Udp()
         {
             udp_logger = Logger::CreateUnique("UDPLogger", GGPO_DEFAULT_LOGGER_FLAGS, GGPO_DEFAULT_LOG_OUTPUT_DIRECTORY);
            GGPO_ASSERT(udp_logger) 
         }

         void
             Init(uint16_t port, Poll* poll, Callbacks* callbacks)
         {
             _callbacks = callbacks;

             _poll = poll;
             _poll->RegisterLoop(this);

             udp_logger->Info(format("binding udp socket to port {}.", port), "udp.cpp");
             _socket = CreateSocket(port, 0, udp_logger.get());
         }

         void
             SendTo
             (
                 char* buffer, 
                 int len, 
                 int flags, 
                 struct sockaddr* dst,
                 int destlen
             )
         {
             struct sockaddr_in* to = (struct sockaddr_in*)dst;

             int res = sendto(_socket, buffer, len, flags, dst, destlen);

             if (res == GGPO_SOCKET_ERROR)
             {
                 GGPO_NETWORK_ERROR_CODE err = GGPO_GET_LAST_ERROR();
                 udp_logger->Error(format("unknown error in sendto (erro: {}  wsaerr: {}).", res, err), "udp.cpp");
                 GGPO_ASSERT(FALSE && "Unknown error in sendto");
             }

             char dst_ip[1024];

             udp_logger->Error(format("sent packet length {} to {}:{} (ret:{}).", len, inet_ntop(AF_INET, (void*)&to->sin_addr, dst_ip, GGPO_ARRAY_SIZE(dst_ip)), ntohs(to->sin_port), res), "udp.cpp");
         }

         virtual bool
             OnLoopPoll(void* cookie)
         {
             uint8_t recv_buf[MAX_UDP_PACKET_SIZE];
             sockaddr_in recv_addr;
             int recv_addr_len;

             for (;;) //tf is this C style shiznit
             {
                 recv_addr_len = sizeof(recv_addr);
                 int len = recvfrom(_socket, (char*)recv_buf, MAX_UDP_PACKET_SIZE, 0, (struct sockaddr*)&recv_addr, &recv_addr_len);

                 // TODO: handle len == 0... indicates a disconnect.

                 if (len == -1)
                 {
                     int error = GGPO_GET_LAST_ERROR();

                     if (error != GGPO_SOCKET_ERROR_CODE)
                     {
                         udp_logger->Error(format("recvfrom GGPO_GET_LAST_ERROR returned {} ({}).", error, error), "udp.cpp");
                     }

                     break;
                 }
                 else if (len > 0)
                 {
                     char src_ip[1024];
                     udp_logger->Error(format("recvfrom returned (len:{}  from:{}:{}).", len, inet_ntop(AF_INET, (void*)&recv_addr.sin_addr, src_ip, GGPO_ARRAY_SIZE(src_ip)), ntohs(recv_addr.sin_port)), "udp.cpp");
                     UdpMsg* msg = (UdpMsg*)recv_buf;
                     _callbacks->OnMsg(recv_addr, msg, len);
                 }
             }
             return true;
         }

     public:
         ~Udp(void)
         {
             if (_socket != GGPO_INVALID_SOCKET)
             {
                 GGPO_CLOSE_SOCKET(_socket);
                 _socket = GGPO_INVALID_SOCKET;
             }
         }

     protected:
         // Network transmission information
         GGPO_SOCKET _socket = GGPO_INVALID_SOCKET;

         // state management
         Callbacks* _callbacks = NULL;
         Poll* _poll = NULL;

         unique_ptr<Logger> udp_logger = nullptr;

     };
}

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

namespace GGPO
{
    class UdpProtocol
    {
    private:
        unique_ptr<Logger> udp_protocol_logger = nullptr;

    public:
        struct Stats
        {
            int                 ping;
            int                 remote_frame_advantage;
            int                 local_frame_advantage;
            int                 send_queue_len;
            Udp::Stats        udp;
        };

        struct Event 
        {
            enum Type 
            {
                Unknown = -1,
                Connected,
                Synchronizing,
                Synchronzied,
                Input,
                Disconnected,
                NetworkInterrupted,
                NetworkResumed,
            };

            Type      type;
            union 
            {
                struct 
                {
                    GameInput   input;
                } input;
                struct 
                {
                    int         total;
                    int         count;
                } synchronizing;
                struct 
                {
                    int         disconnect_timeout;
                } network_interrupted;
            } u;

            UdpProtocol::Event(Type t = Unknown) : type(t) { }
        };

    public:
        virtual bool
            OnLoopPoll(void* cookie)
        {
            if (not _udp)
            {
                return true;
            }

            unsigned int now = Platform::GetMonotonicTimeMS();
            unsigned int next_interval;

            PumpSendQueue();

            switch (_current_state)
            {
            case Syncing:
                next_interval = (_state.sync.roundtrips_remaining == NUM_SYNC_PACKETS) ? SYNC_FIRST_RETRY_INTERVAL : SYNC_RETRY_INTERVAL;
                if (_last_send_time and _last_send_time + next_interval < now)
                {
                    udp_protocol_logger->Info(format("No luck syncing after {} ms... Re-queueing sync packet.", next_interval), "udp_proto.cpp");
                    SendSyncRequest();
                }
                break;

            case Running:
                // xxx: rig all this up with a timer wrapper
                if (not _state.running.last_input_packet_recv_time or _state.running.last_input_packet_recv_time + RUNNING_RETRY_INTERVAL < now)
                {
                    udp_protocol_logger->Info(format("Haven't exchanged packets in a while (last received:{}  last sent:{}).  Resending.", _last_received_input.frame, _last_sent_input.frame), "udp_proto.cpp");
                    SendPendingOutput();
                    _state.running.last_input_packet_recv_time = now;
                }

                if (not _state.running.last_quality_report_time or _state.running.last_quality_report_time + QUALITY_REPORT_INTERVAL < now)
                {
                    UdpMsg* msg = new UdpMsg(UdpMsg::QualityReport);
                    msg->u.quality_report.ping = Platform::GetMonotonicTimeMS();
                    msg->u.quality_report.frame_advantage = (uint8_t)_local_frame_advantage;
                    SendMsg(msg);
                    _state.running.last_quality_report_time = now;
                }

                if (not _state.running.last_network_stats_interval or _state.running.last_network_stats_interval + NETWORK_STATS_INTERVAL < now)
                {
                    UpdateNetworkStats();
                    _state.running.last_network_stats_interval = now;
                }

                if (_last_send_time and _last_send_time + KEEP_ALIVE_INTERVAL < now)
                {
                    udp_protocol_logger->Info("Sending keep alive packet", "udp_proto.cpp");
                    SendMsg(new UdpMsg(UdpMsg::KeepAlive));
                }

                if
                    (
                        _disconnect_timeout and
                        _disconnect_notify_start and
                        not _disconnect_notify_sent and
                        (_last_recv_time + _disconnect_notify_start < now)
                        )
                {
                    udp_protocol_logger->Info(format("Endpoint has stopped receiving packets for {} ms.  Sending notification.", _disconnect_notify_start), "udp_proto.cpp");
                    Event e(Event::NetworkInterrupted);
                    e.u.network_interrupted.disconnect_timeout = _disconnect_timeout - _disconnect_notify_start;
                    QueueEvent(e);
                    _disconnect_notify_sent = true;
                }

                if (_disconnect_timeout and (_last_recv_time + _disconnect_timeout < now))
                {
                    if (not _disconnect_event_sent)
                    {
                        udp_protocol_logger->Info(format("Endpoint has stopped receiving packets for {} ms.  Disconnecting.", _disconnect_timeout), "udp_proto.cpp");
                        QueueEvent(Event(Event::Disconnected));
                        _disconnect_event_sent = true;
                    }
                }
                break;

            case Disconnected:
                if (_shutdown_timeout < now)
                {
                    udp_protocol_logger->Info("Shutting down udp connection.", "udp_proto.cpp");
                    _udp = NULL;
                    _shutdown_timeout = 0;
                }
            }

            return true;
        }

    public:
        UdpProtocol() :
            _local_frame_advantage(0),
            _remote_frame_advantage(0),
            _queue(-1),
            _magic_number(0),
            _remote_magic_number(0),
            _packets_sent(0),
            _bytes_sent(0),
            _stats_start_time(0),
            _last_send_time(0),
            _shutdown_timeout(0),
            _disconnect_timeout(0),
            _disconnect_notify_start(0),
            _disconnect_notify_sent(false),
            _disconnect_event_sent(false),
            _connected(false),
            _next_send_seq(0),
            _next_recv_seq(0),
            _udp(NULL)
        {
            udp_protocol_logger = Logger::CreateUnique("UDPProtocolLogger", GGPO_DEFAULT_LOGGER_FLAGS, GGPO_DEFAULT_LOG_OUTPUT_DIRECTORY);

            GGPO_ASSERT(udp_protocol_logger)

            _last_sent_input.init(-1, NULL, 1);
            _last_received_input.init(-1, NULL, 1);
            _last_acked_input.init(-1, NULL, 1);

            memset(&_state, 0, sizeof _state);
            memset(_peer_connect_status, 0, sizeof(_peer_connect_status));
            for (int i = 0; i < GGPO_ARRAY_SIZE(_peer_connect_status); i++) {
                _peer_connect_status[i].last_frame = -1;
            }
            memset(&_peer_addr, 0, sizeof _peer_addr);
            _oo_packet.msg = NULL;

            _send_latency = Platform::GetConfigInt("ggpo.network.delay");
            _oop_percent = Platform::GetConfigInt("ggpo.oop.percent");
        }

        virtual 
            ~UdpProtocol()
        {
            ClearSendQueue();
        }

        void
            Init
            (
                Udp* udp,
                Poll& poll,
                int queue,
                char* ip,
                uint16_t port,
                UdpMsg::connect_status* status
            )
        {
            _udp = udp;
            _queue = queue;
            _local_connect_status = status;

            _peer_addr.sin_family = AF_INET;
            _peer_addr.sin_port = htons(port);
            inet_pton(AF_INET, ip, &_peer_addr.sin_addr.s_addr);

            do {
                _magic_number = (uint16_t)rand();
            } while (_magic_number == 0);
            poll.RegisterLoop(this);
        }

        void
            Synchronize()
        {
            if (_udp)
            {
                _current_state = Syncing;
                _state.sync.roundtrips_remaining = NUM_SYNC_PACKETS;
                SendSyncRequest();
            }
        }

        bool
            GetPeerConnectStatus(int id, int* frame)
        {
            *frame = _peer_connect_status[id].last_frame;
            return !_peer_connect_status[id].disconnected;
        }

        void
            SendInput(GameInput& input)
        {
            if (_udp)
            {
                if (_current_state == Running)
                {
                    /*
                     * Check to see if this is a good time to adjust for the rift...
                     */
                    _timesync.advance_frame(input, _local_frame_advantage, _remote_frame_advantage);

                    /*
                     * Save this input packet
                     *
                     * XXX: This queue may fill up for spectators who do not ack input packets in a timely
                     * manner.  When this happens, we can either resize the queue (ug) or disconnect them
                     * (better, but still ug).  For the meantime, make this queue really big to decrease
                     * the odds of this happening...
                     */
                    _pending_output.TryPush(input);
                }
                SendPendingOutput();
            }
        }

        void
            SendInputAck()
        {
            UdpMsg* msg = new UdpMsg(UdpMsg::InputAck);
            msg->u.input_ack.ack_frame = _last_received_input.frame;
            SendMsg(msg);
        }

        bool
            HandlesMsg
            (
                sockaddr_in& from,
                UdpMsg* msg
            )
        {
            if (not _udp)
            {
                return false;
            }

            return _peer_addr.sin_addr.S_un.S_addr == from.sin_addr.S_un.S_addr and
                _peer_addr.sin_port == from.sin_port;
        }

        void
            OnMsg(UdpMsg* msg, int len)
        {
            bool handled = false;

            typedef bool (UdpProtocol::* DispatchFn)(UdpMsg* msg, int len);

            static const DispatchFn table[] =
            {
               &UdpProtocol::OnInvalid,             /* Invalid */
               &UdpProtocol::OnSyncRequest,         /* SyncRequest */
               &UdpProtocol::OnSyncReply,           /* SyncReply */
               &UdpProtocol::OnInput,               /* Input */
               &UdpProtocol::OnQualityReport,       /* QualityReport */
               &UdpProtocol::OnQualityReply,        /* QualityReply */
               &UdpProtocol::OnKeepAlive,           /* KeepAlive */
               &UdpProtocol::OnInputAck,            /* InputAck */
            };

            // filter out messages that don't match what we expect
            uint16_t seq = msg->hdr.sequence_number;

            if (msg->hdr.type != UdpMsg::SyncRequest and msg->hdr.type != UdpMsg::SyncReply)
            {
                if (msg->hdr.magic != _remote_magic_number)
                {
                    LogMsg("recv rejecting", msg);
                    return;
                }

                // filter out out-of-order packets
                uint16_t skipped = (uint16_t)((int)seq - (int)_next_recv_seq);
                // Log("checking sequence number -> next - seq : %d - %d = %d\n", seq, _next_recv_seq, skipped);
                if (skipped > MAX_SEQ_DISTANCE)
                {
                    udp_protocol_logger->Info(format("dropping out of order packet (seq: {}, last seq:{})", seq, _next_recv_seq), "udp_proto.cpp");
                    return;
                }
            }

            _next_recv_seq = seq;
            LogMsg("recv", msg);

            if (msg->hdr.type >= GGPO_ARRAY_SIZE(table))
            {
                OnInvalid(msg, len);
            }
            else
            {
                handled = (this->*(table[msg->hdr.type]))(msg, len);
            }
            if (handled)
            {
                _last_recv_time = Platform::GetMonotonicTimeMS();

                if (_disconnect_notify_sent and _current_state == Running)
                {
                    QueueEvent(Event(Event::NetworkResumed));
                    _disconnect_notify_sent = false;
                }
            }
        }

        void
            Disconnect()
        {
            _current_state = Disconnected;
            _shutdown_timeout = Platform::GetMonotonicTimeMS() + UDP_SHUTDOWN_TIMER;
        }

        void
            GetNetworkStats(struct NetworkStats* s) //wow great variable name tony GOOD FUCKING NAME
        {
            s->network.ping = _round_trip_time;
            s->network.send_queue_len = _pending_output.CurrentSize();
            s->network.kbps_sent = _kbps_sent;
            s->timesync.remote_frames_behind = _remote_frame_advantage;
            s->timesync.local_frames_behind = _local_frame_advantage;
        }

        bool
            GetEvent(UdpProtocol::Event& e)
        {
            if (_event_queue.CurrentSize() == 0)
            {
                return false;
            }

            e = _event_queue.Front();
            _event_queue.Pop();

            return true;
        }

        void
            SetLocalFrameNumber(int localFrame)
        {
            /*
             * Estimate which frame the other guy is one by looking at the
             * last frame they gave us plus some delta for the one-way packet
             * trip time.
             */
            int remoteFrame = _last_received_input.frame + (_round_trip_time * 60 / 1000);

            /*
             * Our frame advantage is how many frames *behind* the other guy
             * we are.  Counter-intuative, I know.  It's an advantage because
             * it means they'll have to predict more often and our moves will
             * pop more frequenetly.
             */
            _local_frame_advantage = remoteFrame - localFrame;
        }

        int
            RecommendFrameDelay()
        {
            // XXX: require idle input should be a configuration parameter
            return _timesync.recommend_frame_wait_duration(false, udp_protocol_logger.get());
        }

        void
            SetDisconnectTimeout(int timeout)
        {
            _disconnect_timeout = timeout;
        }

        void
            SetDisconnectNotifyStart(int timeout)
        {
            _disconnect_notify_start = timeout;
        }

        bool IsInitialized()
        {
            return _udp != NULL;
        }

        bool
            IsSynchronized()
            const
        {
            return _current_state == Running;
        }

        bool
            IsRunning()
            const
        {
            return _current_state == Running;
        }

    protected:
        enum State
        {
            Syncing,
            Synchronzied,
            Running,
            Disconnected
        };

        struct QueueEntry
        {
            int queue_time = -1;
            sockaddr_in dest_addr;
            UdpMsg* msg = nullptr;

            QueueEntry() {}
            QueueEntry(int time, sockaddr_in& dst, UdpMsg* m) : queue_time(time), dest_addr(dst), msg(m) { }
        };

        void
            UpdateNetworkStats(void)
        {
            int now = Platform::GetMonotonicTimeMS();

            if (_stats_start_time == 0)
            {
                _stats_start_time = now;
            }

            int total_bytes_sent = _bytes_sent + (UDP_HEADER_SIZE * _packets_sent);
            float seconds = (float)((now - _stats_start_time) / 1000.0);
            float Bps = total_bytes_sent / seconds;
            float udp_overhead = (float)(100.0 * (UDP_HEADER_SIZE * _packets_sent) / _bytes_sent);

            _kbps_sent = int(Bps / 1024);

            udp_protocol_logger->Info
            (
                format
                (
                    "Network Stats -- Bandwidth: {} KBps   Packets Sent: {} ({} pps)   " "KB Sent: {}    UDP Overhead: {}.",
                    _kbps_sent,
                    _packets_sent,
                    (float)_packets_sent * 1000 / (now - _stats_start_time),
                    total_bytes_sent / 1024.0,
                    udp_overhead
                ),
                "udp_proto.cpp"
            );
        }

        void
            QueueEvent(const UdpProtocol::Event& evt)
        {
            LogEvent("Queuing event", evt);
            _event_queue.SafePush(evt);
        }

        void
            ClearSendQueue(void)
        {
            while (not _send_queue.IsEmpty())
            {
                delete _send_queue.Front().msg;
                _send_queue.Pop();
            }
        }

        void
            LogMsg
            (
                const char* prefix,
                UdpMsg* msg
            )
        {
            switch (msg->hdr.type)
            {
            case UdpMsg::SyncRequest:
                udp_protocol_logger->Info(format("{} sync-request ({}).", prefix, msg->u.sync_request.random_request), "udp_proto.cpp");
                break;

            case UdpMsg::SyncReply:
                udp_protocol_logger->Info(format("{} sync-reply ({}).", prefix, msg->u.sync_reply.random_reply), "udp_proto.cpp");
                break;

            case UdpMsg::QualityReport:
                udp_protocol_logger->Info(format("{} quality report.", prefix), "udp_proto.cpp");
                break;

            case UdpMsg::QualityReply:
                udp_protocol_logger->Info(format("{} quality reply.", prefix), "udp_proto.cpp");
                break;

            case UdpMsg::KeepAlive:
                udp_protocol_logger->Info(format("{} keep alive.", prefix), "udp_proto.cpp");
                break;

            case UdpMsg::Input:
                udp_protocol_logger->Info(format("{} game-compressed-input {} (+ {} bits).", prefix, msg->u.input.start_frame, msg->u.input.num_bits), "udp_proto.cpp");
                break;

            case UdpMsg::InputAck:
                udp_protocol_logger->Info(format("{} input ack.", prefix), "udp_proto.cpp");
                break;

            default:
                GGPO_ASSERT(false and "Unknown UdpMsg type.");
            }
        }

        void
            LogEvent
            (
                const char* prefix, 
                const UdpProtocol::Event& evt
            )
        {
            switch (evt.type)
            {
            case UdpProtocol::Event::Synchronzied:
                udp_protocol_logger->Info(format("{} (event: Synchronzied).", prefix), "udp_proto.cpp");
                break;
            }
        }

        void
            SendSyncRequest()
        {
            _state.sync.random = rand() & 0xFFFF;
            UdpMsg* msg = new UdpMsg(UdpMsg::SyncRequest);
            msg->u.sync_request.random_request = _state.sync.random;
            SendMsg(msg);
        }

        void
            SendMsg(UdpMsg* msg)
        {
            LogMsg("send", msg);

            _packets_sent++;
            _last_send_time = Platform::GetMonotonicTimeMS();
            _bytes_sent += msg->PacketSize();

            msg->hdr.magic = _magic_number;
            msg->hdr.sequence_number = _next_send_seq++;

            _send_queue.SafePush(QueueEntry(Platform::GetMonotonicTimeMS(), _peer_addr, msg));
            PumpSendQueue();
        }

        void
            PumpSendQueue()
        {
            while (not _send_queue.IsEmpty())
            {
                QueueEntry& entry = _send_queue.Front();

                if (_send_latency)
                {
                    // should really come up with a gaussian distributation based on the configured
                    // value, but this will do for now.
                    int jitter = (_send_latency * 2 / 3) + ((rand() % _send_latency) / 3);

                    if (Platform::GetMonotonicTimeMS() < _send_queue.Front().queue_time + jitter)
                    {
                        break;
                    }
                }
                if (_oop_percent and not _oo_packet.msg and ((rand() % 100) < _oop_percent))
                {
                    int delay = rand() % (_send_latency * 10 + 1000);

                    if (entry.msg) //check for a nullptr dereference
                    {
                        udp_protocol_logger->Info(format("creating rogue oop (seq: {}  delay: {})", entry.msg->hdr.sequence_number, delay), "udp_proto.cpp");
                    }
                    else
                    {
                        //???????????????????????????????????????????????????????? WHY IS THIS HERE TONY ANSWER ME
                    }

                    _oo_packet.send_time = Platform::GetMonotonicTimeMS() + delay;
                    _oo_packet.msg = entry.msg;
                    _oo_packet.dest_addr = entry.dest_addr;
                }
                else
                {
                    GGPO_ASSERT(entry.dest_addr.sin_addr.s_addr);

                    _udp->SendTo((char*)entry.msg, entry.msg->PacketSize(), 0, (struct sockaddr*)&entry.dest_addr, sizeof entry.dest_addr);

                    delete entry.msg;
                }

                _send_queue.Pop();
            }
            if (_oo_packet.msg and _oo_packet.send_time < Platform::GetMonotonicTimeMS())
            {
                udp_protocol_logger->Info("sending rogue oop!", "udp_proto.cpp");

                _udp->SendTo((char*)_oo_packet.msg, _oo_packet.msg->PacketSize(), 0, (struct sockaddr*)&_oo_packet.dest_addr, sizeof _oo_packet.dest_addr);

                delete _oo_packet.msg;
                _oo_packet.msg = NULL;
            }
        }

        void
            SendPendingOutput()
        {
            UdpMsg* msg = new UdpMsg(UdpMsg::Input);
            int i, j, offset = 0;
            uint8_t* bits;
            GameInput last;

            if (_pending_output.CurrentSize())
            {
                last = _last_acked_input;
                bits = msg->u.input.bits;

                msg->u.input.start_frame = _pending_output.Front().frame;
                msg->u.input.input_size = (uint8_t)_pending_output.Front().size;

                GGPO_ASSERT(last.frame == -1 || last.frame + 1 == msg->u.input.start_frame);

                for (j = 0; j < _pending_output.CurrentSize(); j++)
                {
                    GameInput& current = _pending_output.At(j);

                    if (memcmp(current.bits, last.bits, current.size) != 0)
                    {
                        GGPO_ASSERT((GAMEINPUT_MAX_BYTES * GAMEINPUT_MAX_PLAYERS * 8) < (1 << GGPO_BITVECTOR_NIBBLE_SIZE));

                        for (i = 0; i < current.size * 8; i++)
                        {
                            GGPO_ASSERT(i < (1 << GGPO_BITVECTOR_NIBBLE_SIZE));

                            if (current.value(i) != last.value(i))
                            {
                                BitVector_SetBit(msg->u.input.bits, &offset);
                                (current.value(i) ? BitVector_SetBit : BitVector_ClearBit)(bits, &offset);
                                BitVector_WriteNibblet(bits, i, &offset);
                            }
                        }
                    }
                    BitVector_ClearBit(msg->u.input.bits, &offset);
                    last = _last_sent_input = current;
                }
            }
            else
            {
                msg->u.input.start_frame = 0;
                msg->u.input.input_size = 0;
            }

            msg->u.input.ack_frame = _last_received_input.frame;
            msg->u.input.num_bits = (uint16_t)offset;

            msg->u.input.disconnect_requested = _current_state == Disconnected;

            if (_local_connect_status)
            {
                memcpy(msg->u.input.peer_connect_status, _local_connect_status, sizeof(UdpMsg::connect_status) * UDP_MSG_MAX_PLAYERS);
            }
            else
            {
                memset(msg->u.input.peer_connect_status, 0, sizeof(UdpMsg::connect_status) * UDP_MSG_MAX_PLAYERS);
            }

            GGPO_ASSERT(offset < MAX_COMPRESSED_BITS);

            SendMsg(msg);
        }

        bool
            OnInvalid(UdpMsg* msg, int len)
        {
            GGPO_ASSERT(false and "Invalid msg in UdpProtocol");
            return false;
        }

        bool
            OnSyncRequest(UdpMsg* msg, int len)
        {
            if (_remote_magic_number != 0 and msg->hdr.magic != _remote_magic_number)
            {
                udp_protocol_logger->Error(format("Ignoring sync request from unknown endpoint ({} != {}).", msg->hdr.magic, _remote_magic_number), "udp_proto.cpp");
                return false;
            }

            UdpMsg* reply = new UdpMsg(UdpMsg::SyncReply);
            reply->u.sync_reply.random_reply = msg->u.sync_request.random_request;
            SendMsg(reply);

            return true;
        }

        bool
            OnSyncReply(UdpMsg* msg, int len)
        {
            if (_current_state != Syncing)
            {
                udp_protocol_logger->Info("Ignoring SyncReply while not synching.", "udp_proto.cpp");
                return msg->hdr.magic == _remote_magic_number;
            }

            if (msg->u.sync_reply.random_reply != _state.sync.random)
            {
                udp_protocol_logger->Info(format("sync reply {} != {}.  Keep looking...", msg->u.sync_reply.random_reply, _state.sync.random), "udp_proto.cpp");
                return false;
            }

            if (not _connected)
            {
                QueueEvent(Event(Event::Connected));
                _connected = true;
            }

            udp_protocol_logger->Info(format("Checking sync state ({} round trips remaining).", _state.sync.roundtrips_remaining), "udp_proto.cpp");

            if (--_state.sync.roundtrips_remaining == 0)
            {
                udp_protocol_logger->Info("Synchronized!", "udp_proto.cpp");
                QueueEvent(UdpProtocol::Event(UdpProtocol::Event::Synchronzied));
                _current_state = Running;
                _last_received_input.frame = -1;
                _remote_magic_number = msg->hdr.magic;
            }
            else
            {
                UdpProtocol::Event evt(UdpProtocol::Event::Synchronizing);
                evt.u.synchronizing.total = NUM_SYNC_PACKETS;
                evt.u.synchronizing.count = NUM_SYNC_PACKETS - _state.sync.roundtrips_remaining;
                QueueEvent(evt);
                SendSyncRequest();
            }

            return true;
        }

        bool
            OnInput(UdpMsg* msg, int len)
        {
            /*
             * If a disconnect is requested, go ahead and disconnect now.
             */
            bool disconnect_requested = msg->u.input.disconnect_requested;

            if (disconnect_requested)
            {
                if (_current_state != Disconnected and not _disconnect_event_sent)
                {
                    udp_protocol_logger->Info("Disconnecting endpoint on remote request.", "udp_proto.cpp");
                    QueueEvent(Event(Event::Disconnected));
                    _disconnect_event_sent = true;
                }
            }
            else
            {
                /*
                 * Update the peer connection status if this peer is still considered to be part
                 * of the network.
                 */
                UdpMsg::connect_status* remote_status = msg->u.input.peer_connect_status;

                for (int i = 0; i < GGPO_ARRAY_SIZE(_peer_connect_status); i++)
                {
                    GGPO_ASSERT(remote_status[i].last_frame >= _peer_connect_status[i].last_frame);

                    _peer_connect_status[i].disconnected = _peer_connect_status[i].disconnected || remote_status[i].disconnected;
                    _peer_connect_status[i].last_frame = GGPO_MAX(_peer_connect_status[i].last_frame, remote_status[i].last_frame);
                }
            }

            /*
             * Decompress the input.
             */
            int last_received_frame_number = _last_received_input.frame;

            if (msg->u.input.num_bits)
            {
                int offset = 0;
                uint8_t* bits = (uint8_t*)msg->u.input.bits;
                int numBits = msg->u.input.num_bits;
                int currentFrame = msg->u.input.start_frame;

                _last_received_input.size = msg->u.input.input_size;

                if (_last_received_input.frame < 0)
                {
                    _last_received_input.frame = msg->u.input.start_frame - 1;
                }

                while (offset < numBits)
                {
                    /*
                     * Keep walking through the frames (parsing bits) until we reach
                     * the inputs for the frame right after the one we're on.
                     */
                    GGPO_ASSERT(currentFrame <= (_last_received_input.frame + 1));
                    bool useInputs = currentFrame == _last_received_input.frame + 1;

                    while (BitVector_ReadBit(bits, &offset))
                    {
                        int on = BitVector_ReadBit(bits, &offset);
                        int button = BitVector_ReadNibblet(bits, &offset);

                        if (useInputs)
                        {
                            if (on)
                            {
                                _last_received_input.set(button);
                            }
                            else
                            {
                                _last_received_input.clear(button);
                            }
                        }
                    }
                    GGPO_ASSERT(offset <= numBits);

                    /*
                     * Now if we want to use these inputs, go ahead and send them to
                     * the emulator.
                     */
                    if (useInputs)
                    {
                        /*
                         * Move forward 1 frame in the stream.
                         */
                        char desc[1024];
                        GGPO_ASSERT(currentFrame == _last_received_input.frame + 1);
                        _last_received_input.frame = currentFrame;

                        /*
                         * Send the event to the emualtor
                         */
                        UdpProtocol::Event evt(UdpProtocol::Event::Input);
                        evt.u.input.input = _last_received_input;

                        _last_received_input.Description(desc);

                        _state.running.last_input_packet_recv_time = Platform::GetMonotonicTimeMS();

                        udp_protocol_logger->Info(format("Sending frame {} to emu queue {} ({}).", _last_received_input.frame, _queue, desc), "udp_proto.cpp");
                        QueueEvent(evt);

                    }
                    else
                    {
                        udp_protocol_logger->Info(format("Skipping past frame:({}) current is {}.", currentFrame, _last_received_input.frame), "udp_proto.cpp");
                    }

                    /*
                     * Move forward 1 frame in the input stream.
                     */
                    currentFrame++;
                }
            }

            GGPO_ASSERT(_last_received_input.frame >= last_received_frame_number);

            /*
             * Get rid of our buffered input
             */
            while (_pending_output.CurrentSize() and _pending_output.Front().frame < msg->u.input.ack_frame)
            {
                udp_protocol_logger->Info(format("Throwing away pending output frame {}", _pending_output.Front().frame), "udp_proto.cpp");
                _last_acked_input = _pending_output.Front();
                _pending_output.Pop();
            }
            return true;
        }

        bool
            OnInputAck(UdpMsg* msg, int len)
        {
            /*
             * Get rid of our buffered input
             */
            while (_pending_output.CurrentSize() and _pending_output.Front().frame < msg->u.input_ack.ack_frame)
            {
                udp_protocol_logger->Info(format("Throwing away pending output frame {}", _pending_output.Front().frame), "udp_proto.cpp");
                _last_acked_input = _pending_output.Front();
                _pending_output.Pop();
            }

            return true;
        }

        bool
            OnQualityReport(UdpMsg* msg, int len)
        {
            // send a reply so the other side can compute the round trip transmit time.
            UdpMsg* reply = new UdpMsg(UdpMsg::QualityReply);
            reply->u.quality_reply.pong = msg->u.quality_report.ping;
            SendMsg(reply);

            _remote_frame_advantage = msg->u.quality_report.frame_advantage;
            return true;
        }

        bool
            OnQualityReply(UdpMsg* msg, int len)
        {
            _round_trip_time = Platform::GetMonotonicTimeMS() - msg->u.quality_reply.pong;
            return true;
        }

        bool
           OnKeepAlive(UdpMsg* msg, int len) //wtf?
        {
            return true;
        }

    protected:
        /*
        * Network transmission information
        */
        Udp* _udp;
        sockaddr_in    _peer_addr;
        uint16_t         _magic_number;
        int            _queue;
        uint16_t         _remote_magic_number;
        bool           _connected;
        int            _send_latency;
        int            _oop_percent;
        struct 
        {
            int         send_time = -1;
            sockaddr_in dest_addr;
            UdpMsg* msg = nullptr;
        } _oo_packet;
        RingBuffer<QueueEntry, 64> _send_queue;

        /*
        * Stats
        */
        int            _round_trip_time;
        int            _packets_sent;
        int            _bytes_sent;
        int            _kbps_sent;
        int            _stats_start_time;

        /*
        * The state machine
        */
        UdpMsg::connect_status* _local_connect_status;
        UdpMsg::connect_status _peer_connect_status[UDP_MSG_MAX_PLAYERS];

        State          _current_state;

        union 
        {
            struct 
            {
                uint32_t   roundtrips_remaining;
                uint32_t   random;
            } sync;
            struct 
            {
                uint32_t   last_quality_report_time;
                uint32_t   last_network_stats_interval;
                uint32_t   last_input_packet_recv_time;
            } running;
        } _state;

        /*
        * Fairness.
        */
        int               _local_frame_advantage;
        int               _remote_frame_advantage;

        /*
        * Packet loss...
        */
        RingBuffer<GameInput, 64>  _pending_output;
        GameInput                  _last_received_input;
        GameInput                  _last_sent_input;
        GameInput                  _last_acked_input;
        unsigned int               _last_send_time;
        unsigned int               _last_recv_time;
        unsigned int               _shutdown_timeout;
        unsigned int               _disconnect_event_sent;
        unsigned int               _disconnect_timeout;
        unsigned int               _disconnect_notify_start;
        bool                       _disconnect_notify_sent;

        uint16_t                     _next_send_seq;
        uint16_t                     _next_recv_seq;

        /*
        * Rift synchronization.
        */
        TimeSync                   _timesync;

        /*
        * Event queue
        */
        RingBuffer<UdpProtocol::Event, 64>  _event_queue;
    };
}
 
//==================================================================================== SyncTest Backend ============================================================================================//

namespace GGPO
{
    class SyncTestBackend
    {
    private:
        unique_ptr<Logger> sync_test_backend_logger = nullptr;

     public:
         SyncTestBackend
         (
             string gamename,
             const int frames,
             const int num_players
         ) :
             _sync(NULL)
         {
             sync_test_backend_logger = Logger::CreateUnique("SyncTestBackendLogger", GGPO_DEFAULT_LOGGER_FLAGS, GGPO_DEFAULT_LOG_OUTPUT_DIRECTORY);
             
             GGPO_ASSERT(sync_test_backend_logger)

             _num_players = num_players;
             _check_distance = frames;
             _last_verified = 0;
             _rollingback = false;
             _running = false;
             _current_input.erase();
             pm_GameName = gamename;

             /*
              * Initialize the synchronziation layer
              */
             Sync::Config config = { 0 };
             config.num_prediction_frames = MAX_PREDICTION_FRAMES;
             _sync.Init(config);

             /*
              * Preload the ROM
              */
              //_callbacks.begin_game(gamename); ?????????????????????????????
         }

         virtual ~SyncTestBackend() = default;

         virtual ErrorCode
             DoPoll()
         {
             if (not _running)
             {
                 Event info;

                 info.code = EventCode::Running;
                 _callbacks.on_event(&info);
                 _running = true;
             }

             return ErrorCode::OK;
         }

         virtual ErrorCode
             AddPlayer
             (
                 Player* player, 
                 PlayerHandle* handle
             )
         {
             if (player->player_num < 1 || player->player_num > _num_players)
             {
                 return ErrorCode::PLAYER_OUT_OF_RANGE;
             }

             *handle = (PlayerHandle)(player->player_num - 1);
             return ErrorCode::OK;
         }

         virtual ErrorCode
             AddLocalInput(PlayerHandle player, void* values, int size)
         {
             if (not _running)
             {
                 return ErrorCode::NOT_SYNCHRONIZED;
             }

             int index = (int)player;

             for (int i = 0; i < size; i++)
             {
                 _current_input.bits[(index * size) + i] |= ((char*)values)[i];
             }

             return ErrorCode::OK;
         }

         virtual ErrorCode
             SyncInput
             (
                 void* values,
                 int size,
                 int* disconnect_flags
             )
         {
             if (_rollingback) //HERES THE THING
             {
                 _last_input = _saved_frames.Front().input;
             }
             else
             {
                 if (_sync.GetFrameCount() == 0)
                 {
                     _sync.SaveCurrentFrame();
                 }
                 _last_input = _current_input;
             }

             memcpy(values, _last_input.bits, size);

             if (disconnect_flags)
             {
                 *disconnect_flags = 0;
             }

             return ErrorCode::OK;
         }

         virtual ErrorCode
             IncrementFrame(void)
         {
             _sync.IncrementFrame();
             _current_input.erase();

             sync_test_backend_logger->Info(format("End of frame({})...", _sync.GetFrameCount()), "synctest.cpp");

             if (_rollingback)
             {
                 return ErrorCode::OK;
             }

             int frame = _sync.GetFrameCount();
             // Hold onto the current frame in our queue of saved states.  We'll need
             // the checksum later to verify that our replay of the same frame got the
             // same results.
             SavedInfo info;

             info.frame = frame;
             info.input = _last_input;
             info.buf = (_sync.GetLastSavedFrame().buf);
             info.checksum = _sync.GetLastSavedFrame().checksum;

             _saved_frames.SafePush(info);

             if (frame - _last_verified == _check_distance)
             {
                 // We've gone far enough ahead and should now start replaying frames.
                 // Load the last verified frame and set the rollback flag to true.
                 _sync.LoadFrame(_last_verified);

                 _rollingback = true;

                 while (not _saved_frames.IsEmpty())
                 {
                     _callbacks.advance_frame(0);

                     // Verify that the checksumn of this frame is the same as the one in our
                     // list.
                     info = _saved_frames.Front();
                     _saved_frames.Pop();

                     if (info.frame != _sync.GetFrameCount()) //REPLACE THIS WITH AN IMMEDIATE END SESSION CALL INSTEAD OF CRASHING THE ENTIRE PROGRAM LMFAO
                     {
                         sync_test_backend_logger->Error(format("Frame number {} does not match saved frame number {}", info.frame, frame), "synctest.cpp");
                         sync_test_backend_logger->Error(format("Program will now exit with error: {}", ErrorToString(ErrorCode::FATAL_DESYNC)), "synctest.cpp");
                         exit(static_cast<int>(ErrorCode::FATAL_DESYNC)); //RAISESYNC ERRROR WAS HERE
                     }

                     int checksum = _sync.GetLastSavedFrame().checksum;

                     if (info.checksum != checksum)
                     {
                         sync_test_backend_logger->Error(format("Checksum for frame {} does not match saved ({} != {})", frame, checksum, info.checksum), "synctest.cpp");
                         sync_test_backend_logger->Error(format("Program will now exit with error: {}", ErrorToString(ErrorCode::FATAL_DESYNC)), "synctest.cpp");
                         exit(static_cast<int>(ErrorCode::FATAL_DESYNC)); //RAISESYNC ERRROR WAS HERE
                     }

                     sync_test_backend_logger->Info(format("Checksum {} for frame {} matches.", checksum, info.frame), "synctest.cpp");
                 }

                 _last_verified = frame;
                 _rollingback = false;
             }

             return ErrorCode::OK;
         }

     protected:
         struct SavedInfo
         {
             int frame;
             int checksum;
             string buf;
             GameInput input;
         };

         void
             LogSaveStates(SavedInfo& info)
         {
             sync_test_backend_logger->Info(format("state-{}-original and {}", _sync.GetFrameCount(), info.buf), "synctest.cpp");

             sync_test_backend_logger->Info(format("state-{}-replay and {}", _sync.GetFrameCount(), _sync.GetLastSavedFrame().buf), "synctest.cpp");
         }

     protected:
         Sync                   _sync;
         int                    _num_players;
         int                    _check_distance;
         int                    _last_verified;
         bool                   _rollingback;
         bool                   _running;
         string                   pm_GameName;

         GameInput                  _current_input;
         GameInput                  _last_input;
         RingBuffer<SavedInfo, 32>  _saved_frames;
     };
}
 
//==================================================================================== Spectator Backend ============================================================================================//
 
 constexpr int SPECTATOR_FRAME_BUFFER_SIZE = 64;
 
 namespace GGPO
 {
    class SpectatorBackend
    {
    private:
        unique_ptr<Logger> spectator_backend_logger = nullptr;
    public:
        SpectatorBackend
        (
            const char* gamename,
            uint16_t localport,
            int num_players,
            int input_size,
            char* hostip,
            uint16_t hostport
        ) :
            _num_players(num_players),
            _input_size(input_size),
            _next_input_to_send(0)
        {
            spectator_backend_logger = Logger::CreateUnique("SpectatorBackendLogger", GGPO_DEFAULT_LOGGER_FLAGS, GGPO_DEFAULT_LOG_OUTPUT_DIRECTORY);
             
            GGPO_ASSERT(spectator_backend_logger)

            _synchronizing = true;

            for (int i = 0; i < GGPO_ARRAY_SIZE(_inputs); i++)
            {
                _inputs[i].frame = -1;
            }

            /*
            * Initialize the UDP port
            */
            _udp.Init(localport, &_poll, this);

            /*
            * Init the host endpoint
            */
            _host.Init(&_udp, _poll, 0, hostip, hostport, NULL);
            _host.Synchronize();

            /*
            * Preload the ROM
            */
            //_callbacks.begin_game(gamename); //?????????????????????????????????
        }

        ~SpectatorBackend() = default;
 
      public:
          ErrorCode
              DoPoll(int timeout)
          {
              _poll.Pump(0);

              PollUdpProtocolEvents();
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
              // Wait until we've started to return inputs.
              if (_synchronizing)
              {
                  return ErrorCode::NOT_SYNCHRONIZED;
              }

              GameInput& input = _inputs[_next_input_to_send % SPECTATOR_FRAME_BUFFER_SIZE];

              if (input.frame < _next_input_to_send)
              {
                  // Haven't received the input from the host yet.  Wait
                  return ErrorCode::PREDICTION_THRESHOLD;
              }
              if (input.frame > _next_input_to_send)
              {
                  // The host is way way way far ahead of the spectator.  How'd this
                  // happen?  Anyway, the input we need is gone forever.
                  return ErrorCode::GENERAL_FAILURE;
              }

              GGPO_ASSERT(size >= _input_size * _num_players);
              memcpy(values, input.bits, _input_size * _num_players);

              if (disconnect_flags)
              {
                  *disconnect_flags = 0; // xxx: should get them from the host!
              }

              _next_input_to_send++;

              return ErrorCode::OK;
          }

          ErrorCode
              IncrementFrame(void)
          {
              spectator_backend_logger->Info(format("End of frame ({})...", _next_input_to_send - 1), "spectator.cpp");
              DoPoll(0);
              PollUdpProtocolEvents();

              return ErrorCode::OK;
          }
 
      public:
          void
              OnMsg
              (
                  sockaddr_in& from,
                  UdpMsg* msg,
                  int len
              )
          {
              if (_host.HandlesMsg(from, msg))
              {
                  _host.OnMsg(msg, len);
              }
          }
 
      protected:
          ErrorCode
              PollUdpProtocolEvents(RingBuffer<Event, GGPO_DEFAULT_RINGBUFFER_SIZE>& fp_EventQueue)
          {
              UdpProtocol::Event evt;

              while (_host.GetEvent(evt))
              {
                  OnUdpProtocolEvent(evt, fp_EventQueue);
              }
          }
 
          ErrorCode
              OnUdpProtocolEvent
              (
                  UdpProtocol::Event& evt,
                  RingBuffer<Event, GGPO_DEFAULT_RINGBUFFER_SIZE>& fp_EventQueue
              )
          {
              Event info;

              switch (evt.type)
              {
              case UdpProtocol::Event::Connected:
                  info.code = EventCode::ConnectedToPeer;
                  info.u.connected.player = 0;
                  fp_EventQueue.Push(info);
                  break;

              case UdpProtocol::Event::Synchronizing:
                  info.code = EventCode::SynchronizingWithPeer;
                  info.u.synchronizing.player = 0;
                  info.u.synchronizing.count = evt.u.synchronizing.count;
                  info.u.synchronizing.total = evt.u.synchronizing.total;
                  fp_EventQueue.Push(info);
                  break;

              case UdpProtocol::Event::Synchronzied:
                  if (_synchronizing)
                  {
                      info.code = EventCode::SynchronizedWithPeer;
                      info.u.synchronized.player = 0;
                      fp_EventQueue.Push(info);

                      info.code = EventCode::Running;
                      fp_EventQueue.Push(info);
                      _synchronizing = false;
                  }
                  break;

              case UdpProtocol::Event::NetworkInterrupted:
                  info.code = EventCode::ConnectionInterrupted;
                  info.u.connection_interrupted.player = 0;
                  info.u.connection_interrupted.disconnect_timeout = evt.u.network_interrupted.disconnect_timeout;
                  fp_EventQueue.Push(info);
                  break;

              case UdpProtocol::Event::NetworkResumed:
                  info.code = EventCode::ConnectionResumed;
                  info.u.connection_resumed.player = 0;
                  fp_EventQueue.Push(info);
                  break;

              case UdpProtocol::Event::Disconnected:
                  info.code = EventCode::DisconnectedFromPeer;
                  info.u.disconnected.player = 0;
                  fp_EventQueue.Push(info);
                  break;

              case UdpProtocol::Event::Input:
                  GameInput& input = evt.u.input.input;

                  _host.SetLocalFrameNumber(input.frame);
                  _host.SendInputAck();
                  _inputs[input.frame % SPECTATOR_FRAME_BUFFER_SIZE] = input;
                  break;
              }
          }
 
      protected:
          Poll                  _poll;
          Udp                   _udp;
          UdpProtocol           _host;
          bool                  _synchronizing;
          int                   _input_size;
          int                   _num_players;
          int                   _next_input_to_send;
          GameInput             _inputs[SPECTATOR_FRAME_BUFFER_SIZE];
      };
 }

//==================================================================================== P2P Backend ============================================================================================//

static constexpr int RECOMMENDATION_INTERVAL = 240;
static constexpr int DEFAULT_DISCONNECT_TIMEOUT = 5000;
static constexpr int DEFAULT_DISCONNECT_NOTIFY_START = 750;
 
 namespace GGPO
 {
    class Peer2PeerBackend
    {
    private:
        unique_ptr<Logger> p2p_backend_logger = nullptr;

      public:
          Peer2PeerBackend
          (
              const char* gamename,
              uint16_t localport,
              int num_players,
              int input_size
          ) :
              _num_players(num_players),
              _input_size(input_size),
              _sync(_local_connect_status),
              _disconnect_timeout(DEFAULT_DISCONNECT_TIMEOUT),
              _disconnect_notify_start(DEFAULT_DISCONNECT_NOTIFY_START),
              _num_spectators(0),
              _next_spectator_frame(0)
          {
              p2p_backend_logger = Logger::CreateUnique("Peer2PeerBackendLogger", GGPO_DEFAULT_LOGGER_FLAGS, GGPO_DEFAULT_LOG_OUTPUT_DIRECTORY);

              GGPO_ASSERT(p2p_backend_logger) //check for successful creation uwu

              _synchronizing = true;
              _next_recommended_sleep = 0;

              /*
               * Initialize the synchronziation layer
               */
              Sync::Config config = { 0 };
              config.num_players = num_players;
              config.input_size = input_size;
              config.num_prediction_frames = MAX_PREDICTION_FRAMES;
              _sync.Init(config);

              /*
               * Initialize the UDP port
               */
              _udp.Init(localport, &_poll, this);

              _endpoints = new UdpProtocol[_num_players];
              memset(_local_connect_status, 0, sizeof(_local_connect_status));

              for (int i = 0; i < _local_connect_status.size(); i++)
              {
                  _local_connect_status[i].last_frame = -1;
              }

              /*
               * Preload the ROM
               */
          }

          virtual ~Peer2PeerBackend()
          {
              delete[] _endpoints;
          }
 
      public:
          virtual ErrorCode
              DoPoll(const int fp_Timeout)
          {
              if (not _sync.InRollback())
              {
                  _poll.Pump(0);

                  PollUdpProtocolEvents();

                  if (not _synchronizing)
                  {
                      _sync.CheckSimulation(fp_Timeout);

                      // notify all of our endpoints of their local frame number for their
                      // next connection quality report
                      int current_frame = _sync.GetFrameCount();

                      for (int i = 0; i < _num_players; i++)
                      {
                          _endpoints[i].SetLocalFrameNumber(current_frame);
                      }

                      int total_min_confirmed;
                      if (_num_players <= 2)
                      {
                          total_min_confirmed = Poll2Players(current_frame);
                      }
                      else
                      {
                          total_min_confirmed = PollNPlayers(current_frame);
                      }

                      p2p_backend_logger->Info(format("last confirmed frame in p2p backend is {}.", total_min_confirmed), "p2p.cpp");

                      if (total_min_confirmed >= 0)
                      {
                          GGPO_ASSERT(total_min_confirmed != INT_MAX);

                          if (_num_spectators > 0)
                          {
                              while (_next_spectator_frame <= total_min_confirmed)
                              {
                                  p2p_backend_logger->Info(format("pushing frame {} to spectators.", _next_spectator_frame), "p2p.cpp");

                                  GameInput input;
                                  input.frame = _next_spectator_frame;
                                  input.size = _input_size * _num_players;

                                  _sync.GetConfirmedInputs(input.bits, _input_size * _num_players, _next_spectator_frame);

                                  for (int i = 0; i < _num_spectators; i++)
                                  {
                                      _spectators[i].SendInput(input);
                                  }
                                  _next_spectator_frame++;
                              }
                          }
                          p2p_backend_logger->Info(format("setting confirmed frame in sync to {}.", total_min_confirmed), "p2p.cpp");

                          _sync.SetLastConfirmedFrame(total_min_confirmed);
                      }

                      // send timesync notifications if now is the proper time
                      if (current_frame > _next_recommended_sleep)
                      {
                          int interval = 0;

                          for (int i = 0; i < _num_players; i++)
                          {
                              interval = GGPO_MAX(interval, _endpoints[i].RecommendFrameDelay());
                          }

                          if (interval > 0)
                          {
                              Event info;
                              info.code = EventCode::TimeSync;
                              info.u.timesync.frames_ahead = interval;
                              _callbacks.on_event(&info);
                              _next_recommended_sleep = current_frame + RECOMMENDATION_INTERVAL;
                          }
                      }
                  }
              }
              return ErrorCode::OK;
          }

          virtual ErrorCode
              AddPlayer
              (
                  Player* player,
                  PlayerHandle* handle
              )
          {
              if (player->type == PlayerType::Spectator)
              {
                  return AddSpectator(player->u.remote.ip_address, player->u.remote.port);
              }

              int queue = player->player_num - 1;

              if (player->player_num < 1 || player->player_num > _num_players)
              {
                  return ErrorCode::PLAYER_OUT_OF_RANGE;
              }

              *handle = QueueToPlayerHandle(queue);

              if (player->type == PlayerType::Remote)
              {
                  AddRemotePlayer(player->u.remote.ip_address, player->u.remote.port, queue);
              }
              return ErrorCode::OK;
          }

          virtual ErrorCode
              AddLocalInput
              (
                  PlayerHandle player,
                  void* values,
                  int size
              )
          {
              int queue;
              GameInput input;
              ErrorCode result;

              if (_sync.InRollback())
              {
                  return ErrorCode::IN_ROLLBACK;
              }
              if (_synchronizing)
              {
                  return ErrorCode::NOT_SYNCHRONIZED;
              }

              result = PlayerHandleToQueue(player, &queue);

              if (not Succeeded(result))
              {
                  return result;
              }

              input.init(-1, (char*)values, size);

              // Feed the input for the current frame into the synchronzation layer.
              if (not _sync.AddLocalInput(queue, input))
              {
                  return ErrorCode::PREDICTION_THRESHOLD;
              }

              if (input.frame != GameInput::NullFrame)
              {  // xxx: <- comment why this is the case
                 // Update the local connect status state to indicate that we've got a
                 // confirmed local frame for this player.  this must come first so it
                 // gets incorporated into the next packet we send.

                  p2p_backend_logger->Info(format("setting local connect status for local queue {} to {}", queue, input.frame), "p2p.cpp");
                  _local_connect_status[queue].last_frame = input.frame;

                  // Send the input to all the remote players.
                  for (int i = 0; i < _num_players; i++)
                  {
                      if (_endpoints[i].IsInitialized())
                      {
                          _endpoints[i].SendInput(input);
                      }
                  }
              }

              return ErrorCode::OK;
          }

          virtual ErrorCode
              SyncInput
              (
                  void* values,
                  int size,
                  int* disconnect_flags
              )
          {
              int flags;

              // Wait until we've started to return inputs.
              if (_synchronizing)
              {
                  return ErrorCode::NOT_SYNCHRONIZED;
              }
              flags = _sync.SynchronizeInputs(values, size);

              if (not disconnect_flags)
              {
                  p2p_backend_logger->Error("Tried to pass nullptr ref to SyncInput(), nothing was done with disconnect_flags arg", "P2P");
                  return ErrorCode::NULLPTR_PASSED_AS_VALUE;
              }

              *disconnect_flags = flags;

              return ErrorCode::OK;
          }

          virtual ErrorCode
              IncrementFrame(void)
          {
              p2p_backend_logger->Info(format("End of frame ({})...", _sync.GetFrameCount()), "p2p.cpp");
              _sync.IncrementFrame();
              DoPoll(0);
              PollSyncEvents();

              return ErrorCode::OK;
          }

          /*
           * Called only as the result of a local decision to disconnect.  The remote
           * decisions to disconnect are a result of us parsing the peer_connect_settings
           * blob in every endpoint periodically.
           */
          virtual ErrorCode
              DisconnectPlayer(PlayerHandle player)
          {
              int queue;
              ErrorCode result;

              result = PlayerHandleToQueue(player, &queue);

              if (not Succeeded(result))
              {
                  return result;
              }

              if (_local_connect_status[queue].disconnected)
              {
                  return ErrorCode::PLAYER_DISCONNECTED;
              }

              if (not _endpoints[queue].IsInitialized())
              {
                  int current_frame = _sync.GetFrameCount();
                  // xxx: we should be tracking who the local player is, but for now assume
                  // that if the endpoint is not initalized, this must be the local player.
                  p2p_backend_logger->Info(format("Disconnecting local player {} at frame {} by user request.", queue, _local_connect_status[queue].last_frame), "p2p.cpp");
                  for (int i = 0; i < _num_players; i++)
                  {
                      if (_endpoints[i].IsInitialized())
                      {
                          DisconnectPlayerQueue(i, current_frame);
                      }
                  }
              }
              else
              {
                  p2p_backend_logger->Info(format("Disconnecting queue {} at frame {} by user request.", queue, _local_connect_status[queue].last_frame), "p2p.cpp");
                  DisconnectPlayerQueue(queue, _local_connect_status[queue].last_frame);
              }
              return ErrorCode::OK;
          }

          virtual ErrorCode
              GetNetworkStats(NetworkStats* stats, PlayerHandle player)
          {
              int queue;
              ErrorCode result;

              result = PlayerHandleToQueue(player, &queue);

              if (not Succeeded(result))
              {
                  return result;
              }

              memset(stats, 0, sizeof * stats);
              _endpoints[queue].GetNetworkStats(stats);

              return ErrorCode::OK;
          }

          virtual ErrorCode
              SetFrameDelay(PlayerHandle player, int delay)
          {
              int queue;
              ErrorCode result;

              result = PlayerHandleToQueue(player, &queue);

              if (not Succeeded(result))
              {
                  return result;
              }
              _sync.SetFrameDelay(queue, delay);
              return ErrorCode::OK;
          }

          virtual ErrorCode
              SetDisconnectTimeout(int timeout)
          {
              _disconnect_timeout = timeout;
              for (int i = 0; i < _num_players; i++)
              {
                  if (_endpoints[i].IsInitialized())
                  {
                      _endpoints[i].SetDisconnectTimeout(_disconnect_timeout);
                  }
              }
              return ErrorCode::OK;
          }

          virtual ErrorCode
              SetDisconnectNotifyStart(int timeout)
          {
              _disconnect_notify_start = timeout;

              for (int i = 0; i < _num_players; i++)
              {
                  if (_endpoints[i].IsInitialized())
                  {
                      _endpoints[i].SetDisconnectNotifyStart(_disconnect_notify_start);
                  }
              }
              return ErrorCode::OK;
          }
 
      public:
          virtual void
              OnMsg(sockaddr_in& from, UdpMsg* msg, int len)
          {
              for (int i = 0; i < _num_players; i++)
              {
                  if (_endpoints[i].HandlesMsg(from, msg))
                  {
                      _endpoints[i].OnMsg(msg, len);
                      return;
                  }
              }
              for (int i = 0; i < _num_spectators; i++)
              {
                  if (_spectators[i].HandlesMsg(from, msg))
                  {
                      _spectators[i].OnMsg(msg, len);
                      return;
                  }
              }
          }
 
      protected:
        ErrorCode
            PlayerHandleToQueue
            (
                PlayerHandle player, 
                int* queue
            )
        {
            int offset = ((int)player - 1);

            if (offset < 0 or offset >= _num_players)
            {
                return ErrorCode::INVALID_PLAYER_HANDLE;
            }

            *queue = offset;

            return ErrorCode::OK;
        }

        PlayerHandle 
            QueueToPlayerHandle
            (
                int queue
            ) 
        { 
            return (PlayerHandle)(queue + 1); 
        }

          PlayerHandle 
              QueueToSpectatorHandle(int queue) 
          { 
              return (PlayerHandle)(queue + 1000); 
          } /* out of range of the player array, basically */
          
          void
              DisconnectPlayerQueue(int queue, int syncto)
          {
              Event info;
              int framecount = _sync.GetFrameCount();

              _endpoints[queue].Disconnect();

              p2p_backend_logger->Info(format("Changing queue {} local connect status for last frame from {} to {} on disconnect request (current: {}).", queue, _local_connect_status[queue].last_frame, syncto, framecount), "p2p.cpp");

              _local_connect_status[queue].disconnected = 1;
              _local_connect_status[queue].last_frame = syncto;

              if (syncto < framecount)
              {
                  p2p_backend_logger->Info(format("adjusting simulation to account for the fact that {} disconnected @ {}.", queue, syncto), "p2p.cpp");
                  _sync.AdjustSimulation(syncto);
                  p2p_backend_logger->Info("finished adjusting simulation.", "p2p.cpp");
              }

              info.code = EventCode::DisconnectedFromPeer;
              info.u.disconnected.player = QueueToPlayerHandle(queue);
              _callbacks.on_event(&info);

              CheckInitialSync();
          }
          
          void
              PollSyncEvents(void)
          {
              Sync::Event e;

              while (_sync.GetEvent(e)) 
              {
                  OnSyncEvent(e);
              }

              return; //?????????????????????????
          }

          void
              PollUdpProtocolEvents(void)
          {
              UdpProtocol::Event evt;

              for (int i = 0; i < _num_players; i++)
              {
                  while (_endpoints[i].GetEvent(evt))
                  {
                      OnUdpProtocolPeerEvent(evt, i);
                  }
              }
              for (int i = 0; i < _num_spectators; i++)
              {
                  while (_spectators[i].GetEvent(evt))
                  {
                      OnUdpProtocolSpectatorEvent(evt, i);
                  }
              }
          }

          void
              CheckInitialSync(void)
          {
              int i; //umm okay ig lmfao

              if (_synchronizing)
              {
                  // Check to see if everyone is now synchronized.  If so,
                  // go ahead and tell the client that we're ok to accept input.
                  for (i = 0; i < _num_players; i++)
                  {
                      // xxx: IsInitialized() must go... we're actually using it as a proxy for "represents the local player"
                      if (_endpoints[i].IsInitialized() and not _endpoints[i].IsSynchronized() and not _local_connect_status[i].disconnected)
                      {
                          return;
                      }
                  }
                  for (i = 0; i < _num_spectators; i++)
                  {
                      if (_spectators[i].IsInitialized() and not _spectators[i].IsSynchronized())
                      {
                          return;
                      }
                  }

                  Event info;
                  info.code = EventCode::Running;
                  _callbacks.on_event(&info);
                  _synchronizing = false;
              }
          }

          int 
              Poll2Players(int current_frame)
          {
              // discard confirmed frames as appropriate
              int total_min_confirmed = GGPO_MAX_INT;

              for (int i = 0; i < _num_players; i++)
              {
                  bool queue_connected = true;

                  if (_endpoints[i].IsRunning())
                  {
                      int ignore;
                      queue_connected = _endpoints[i].GetPeerConnectStatus(i, &ignore);
                  }
                  if (not _local_connect_status[i].disconnected)
                  {
                      total_min_confirmed = GGPO_MIN(_local_connect_status[i].last_frame, total_min_confirmed);
                  }

                  p2p_backend_logger->Info(format("  local endp: connected = {}, last_received = {}, total_min_confirmed = {}.", not _local_connect_status[i].disconnected, _local_connect_status[i].last_frame, total_min_confirmed), "p2p.cpp");

                  if (not queue_connected && not _local_connect_status[i].disconnected)
                  {
                      p2p_backend_logger->Info(format("disconnecting i {} by remote request.", i), "p2p.cpp");
                      DisconnectPlayerQueue(i, total_min_confirmed);
                  }

                  p2p_backend_logger->Info(format("  total_min_confirmed = {}.", total_min_confirmed), "p2p.cpp");
              }

              return total_min_confirmed;
          }

          int 
              PollNPlayers(int current_frame)
          {
              int i, queue, last_received;

              // discard confirmed frames as appropriate
              int total_min_confirmed = GGPO_MAX_INT;

              for (queue = 0; queue < _num_players; queue++)
              {
                  bool queue_connected = true;
                  int queue_min_confirmed = GGPO_MAX_INT;

                  p2p_backend_logger->Info(format("considering queue {}.", queue), "p2p.cpp");

                  for (i = 0; i < _num_players; i++)
                  {
                      // we're going to do a lot of logic here in consideration of endpoint i.
                      // keep accumulating the minimum confirmed point for all n*n packets and
                      // throw away the rest.
                      if (_endpoints[i].IsRunning())
                      {
                          bool connected = _endpoints[i].GetPeerConnectStatus(queue, &last_received);

                          queue_connected = queue_connected && connected;
                          queue_min_confirmed = GGPO_MIN(last_received, queue_min_confirmed);
                          p2p_backend_logger->Info(format("  endpoint {}: connected = {}, last_received = {}, queue_min_confirmed = {}.", i, connected, last_received, queue_min_confirmed), "p2p.cpp");
                      }
                      else
                      {
                          p2p_backend_logger->Info(format("  endpoint {}: ignoring... not running.", i), "p2p.cpp");
                      }
                  }
                  // merge in our local status only if we're still connected!
                  if (not _local_connect_status[queue].disconnected)
                  {
                      queue_min_confirmed = GGPO_MIN(_local_connect_status[queue].last_frame, queue_min_confirmed);
                  }

                  p2p_backend_logger->Info(format("  local endp: connected = {}, last_received = {}, queue_min_confirmed = {}.", not _local_connect_status[queue].disconnected, _local_connect_status[queue].last_frame, queue_min_confirmed), "p2p.cpp");

                  if (queue_connected)
                  {
                      total_min_confirmed = GGPO_MIN(queue_min_confirmed, total_min_confirmed);
                  }
                  else
                  {
                      // check to see if this disconnect notification is further back than we've been before.  If
                      // so, we need to re-adjust.  This can happen when we detect our own disconnect at frame n
                      // and later receive a disconnect notification for frame n-1.
                      if (not _local_connect_status[queue].disconnected or _local_connect_status[queue].last_frame > queue_min_confirmed)
                      {
                          p2p_backend_logger->Info(format("disconnecting queue {} by remote request.", queue), "p2p.cpp");
                          DisconnectPlayerQueue(queue, queue_min_confirmed);
                      }
                  }

                  p2p_backend_logger->Info(format("  total_min_confirmed = {}.", total_min_confirmed), "p2p.cpp");
              }
              return total_min_confirmed;
          }

          void
              AddRemotePlayer
              (
                  char* ip,
                  uint16_t port,
                  int queue
              )
          {
              /*
               * Start the state machine (xxx: no)
               */
              _synchronizing = true;

              _endpoints[queue].Init(&_udp, _poll, queue, ip, port, _local_connect_status);
              _endpoints[queue].SetDisconnectTimeout(_disconnect_timeout);
              _endpoints[queue].SetDisconnectNotifyStart(_disconnect_notify_start);
              _endpoints[queue].Synchronize();
          }

          ErrorCode 
              AddSpectator
              (
                    char* ip,
                    uint16_t port
              )
          {
              if (_num_spectators == MAX_SPECTATORS)
              {
                  return ErrorCode::TOO_MANY_SPECTATORS;
              }
              /*
               * Currently, we can only add spectators before the game starts.
               */
              if (not _synchronizing)
              {
                  return ErrorCode::INVALID_REQUEST;
              }

              int queue = _num_spectators++;

              _spectators[queue].Init(&_udp, _poll, queue + 1000, ip, port, _local_connect_status);
              _spectators[queue].SetDisconnectTimeout(_disconnect_timeout);
              _spectators[queue].SetDisconnectNotifyStart(_disconnect_notify_start);
              _spectators[queue].Synchronize();

              return ErrorCode::OK;
          }

          virtual void OnSyncEvent(Sync::Event& e) { }

          virtual Event
              OnUdpProtocolEvent(UdpProtocol::Event& evt, PlayerHandle handle)
          {
              Event info;

              switch (evt.type)
              {
                case UdpProtocol::Event::Connected:
                {
                    info.code = EventCode::ConnectedToPeer;
                    info.u.connected.player = handle;
                    return info;
                }
                break;

              case UdpProtocol::Event::Synchronizing:
                  info.code = EventCode::SynchronizingWithPeer;
                  info.u.synchronizing.player = handle;
                  info.u.synchronizing.count = evt.u.synchronizing.count;
                  info.u.synchronizing.total = evt.u.synchronizing.total;
                  return info;
                  break;

              case UdpProtocol::Event::Synchronzied:
                  info.code = EventCode::SynchronizedWithPeer;
                  info.u.synchronized.player = handle;
                  return info;

                  CheckInitialSync();
                  break;

              case UdpProtocol::Event::NetworkInterrupted:
                  info.code = EventCode::ConnectionInterrupted;
                  info.u.connection_interrupted.player = handle;
                  info.u.connection_interrupted.disconnect_timeout = evt.u.network_interrupted.disconnect_timeout;
                  return info;
                  break;

              case UdpProtocol::Event::NetworkResumed:
                  info.code = EventCode::ConnectionResumed;
                  info.u.connection_resumed.player = handle;
                  return info;
                  break;
              }
          }

          virtual void
              OnUdpProtocolPeerEvent(UdpProtocol::Event& evt, int queue)
          {
              OnUdpProtocolEvent(evt, QueueToPlayerHandle(queue));

              switch (evt.type)
              {
              case UdpProtocol::Event::Input:
                  if (not _local_connect_status[queue].disconnected)
                  {
                      int current_remote_frame = _local_connect_status[queue].last_frame;
                      int new_remote_frame = evt.u.input.input.frame;
                      GGPO_ASSERT(current_remote_frame == -1 || new_remote_frame == (current_remote_frame + 1));

                      _sync.AddRemoteInput(queue, evt.u.input.input);
                      // Notify the other endpoints which frame we received from a peer
                      p2p_backend_logger->Info(format("setting remote connect status for queue {} to {}", queue, evt.u.input.input.frame), "p2p.cpp");
                      _local_connect_status[queue].last_frame = evt.u.input.input.frame;
                  }
                  break;

              case UdpProtocol::Event::Disconnected:
                  DisconnectPlayer(QueueToPlayerHandle(queue));
                  break;
              }
          }

          virtual void
              OnUdpProtocolSpectatorEvent
              (
                  UdpProtocol::Event& evt, 
                  int queue
              )
          {
              PlayerHandle handle = QueueToSpectatorHandle(queue);
              OnUdpProtocolEvent(evt, handle);

              Event info;

              switch (evt.type) //why tf is this a switch statement LMFAO
              {
              case UdpProtocol::Event::Disconnected:
                  _spectators[queue].Disconnect();

                  info.code = EventCode::DisconnectedFromPeer;
                  info.u.disconnected.player = handle;
                  _callbacks.on_event(&info);

                  break;
              }
          }
 
      protected:
          Poll                  _poll;
          Sync                  _sync;
          Udp                   _udp;
          UdpProtocol* _endpoints;

          array <UdpProtocol, MAX_SPECTATORS> _spectators = {};

          int                   _num_spectators;
          int                   _input_size;
 
          bool                  _synchronizing;
          int                   _num_players;
          int                   _next_recommended_sleep;
 
          int                   _next_spectator_frame;
          int                   _disconnect_timeout;
          int                   _disconnect_notify_start;
 
          array<UdpMsg::connect_status, UDP_MSG_MAX_PLAYERS> _local_connect_status = {};
      };
 }

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
             StartSession(SessionMode mode) 
         {
             pm_Mode = mode;
             pm_Sync.Init(config);
             if (pm_Mode == SessionMode::SyncTest) {
                 pm_CheckDistance = 60; // e.g. every 60 frames, run verification
             }
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
             const int frame = pm_Sync.GetFrameCount();
             // push saved frame info
             SavedInfo info{
                 frame,
                 pm_Sync.GetLastSavedFrame().checksum,
                 pm_Sync.GetLastSavedFrame().buf
             };
             pm_SavedFrames.SafePush(info);

             if (frame - pm_LastVerified >= pm_CheckDistance) {
                 pm_Sync.LoadFrame(pm_LastVerified);
                 pm_RollingBack = true;

                 while (!pm_SavedFrames.IsEmpty()) {
                     auto info = pm_SavedFrames.Front();
                     pm_SavedFrames.Pop();

                     // Re-run frame and compare checksums
                     int newChecksum = pm_Sync.GetLastSavedFrame().checksum;
                     if (info.checksum != newChecksum) {
                         logger->Error("Desync detected!");
                         return ErrorCode::FATAL_DESYNC;
                     }
                 }

                 pm_LastVerified = frame;
                 pm_RollingBack = false;
             }

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
             pm_Sync.IncrementFrame();

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

         Sync pm_Sync;
         Udp pm_Udp;

         vector<UdpProtocol> pm_Endpoints; //needs to be dynamically sized ig for now since total size of lobby isn't knowable from compile time

         unique_ptr<SyncTestBackend> pm_SyncTestBackend = nullptr;

         unique_ptr<Peer2PeerBackend> pm_P2P = nullptr;
         vector<SpectatorBackend> pm_Spectators;
     };
 }

 //=========================================================================================== END OwO ===========================================================================================//


