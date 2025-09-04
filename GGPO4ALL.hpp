/*******************************************************************
 *                                             GGPO4ALL v0.0.1
 *Created by Ranyodh Mandur - ✨ 2025 and GroundStorm Studios, LLC. - ✨ 2009
 *
 *                         Licensed under the MIT License (MIT).
 *                  For more details, see the LICENSE file or visit:
 *                        https://opensource.org/licenses/MIT
 *
 *            GGPO4ALL is a free open source rollback netcode library
********************************************************************/

#pragma once // :^)

#include <format>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <map>
#include <stdarg.h>
#include <string_view>
#include <stdio.h>
#include <memory.h>
#include <array>
#include <cstddef>
#include <cassert>
#include <utility>
#include <cstring>

#include <thread>

//DO NOT DO "using namespace GGPO", it's bad practice and will bring in the using namespace std bit so if you do that you deserve to be miserable

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace GGPO //I like using namespace std, but ik other ppl don't so this will limit its spread just don't do using namespace GGPO
{
    using namespace std; // >O<
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
if you want to run GGPO4ALL in debug mode simple define or release with a console define this

#define GGPO_USING_CONSOLE 

before including GGPO4ALL anywhere in your code
*/

/*
This is for logging settings, adjust as you wish but buyer beware, doing any changes here could break everything so your choice
*/

#ifdef GGPO_USING_CONSOLE
    #define GGPO_LOG LogAndPrint
#else
    #define GGPO_LOG Log
#endif

constexpr uint32_t MAX_NUMBER_OF_LOGS = 1024;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Utility Functions/Structs
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace GGPO //used for pretty logging for windows consoles, requires GGPO_USING_CONSOLE to be defined since release builds should not be opening console
{
    #if (defined(_WIN32) || defined(_WIN64)) && defined(GGPO_USING_CONSOLE) //this needs to be called at least once at the start of the program to enable ANSI colour codes on windows consoles

    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN

    #include <windows.h>

    static bool
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
}//namespace GGPO

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace GGPO {

    template<typename T, size_t pm_MaxCapacity>
    class RingBuffer
    {
        static_assert(pm_MaxCapacity > 0, "RingBuffer size must be greater than 0");

    public:
        constexpr RingBuffer() = default;

        constexpr void Clear() noexcept
        {
            pm_Head = pm_Tail = pm_Size = 0;
        }

        constexpr void SafePush(const T& value)
        {
            assert(not IsFull() and "RingBuffer overflow (consider overwriting or increasing capacity)");
            Push(value); // Safe push, now guaranteed to not assert
        }

        constexpr void SafePush(T&& value)
        {
            assert(not IsFull() and "RingBuffer overflow (consider overwriting or increasing capacity)");
            Push(move(value)); // Move into buffer
        }

        template<typename... Args>
        constexpr void SafeEmplace(Args&&... args)
        {
            assert(not IsFull() and "RingBuffer overflow (consider overwriting or increasing capacity)");
            pm_Buffer[pm_Head] = T(forward<Args>(args)...);
            pm_Head = (pm_Head + 1) % pm_MaxCapacity;
            ++pm_Size;
        }

        constexpr void ForcePush(const T& value)
        {
            if (IsFull())
            {
                Pop(); // Drop the oldest value to make space
            }

            Push(value); // Safe push, now guaranteed to not assert
        }

        constexpr void ForcePush(T&& value)
        {
            if (IsFull())
            {
                Pop(); // Drop the oldest value
            }

            Push(move(value)); // Move into buffer
        }

        template<typename... Args>
        constexpr void ForceEmplace(Args&&... args)
        {
            if (IsFull())
            {
                Pop(); // Make room
            }

            pm_Buffer[pm_Head] = T(forward<Args>(args)...); // Construct T using args, assign to slot
            pm_Head = (pm_Head + 1) % pm_MaxCapacity;
            ++pm_Size;
        }

        constexpr void Pop()
        {
            assert(not IsEmpty() and "Cannot pop from empty RingBuffer");
            pm_Tail = (pm_Tail + 1) % pm_MaxCapacity;
            --pm_Size;
        }

        [[nodiscard]] constexpr T& Front()
        {
            assert(not IsEmpty() and "Cannot access front of empty RingBuffer");
            return pm_Buffer[pm_Tail];
        }

        [[nodiscard]] constexpr const T& Front() const
        {
            assert(not IsEmpty() and "Cannot access front of empty RingBuffer");
            return pm_Buffer[pm_Tail];
        }

        [[nodiscard]] constexpr T& Back()
        {
            assert(not IsEmpty() and "Cannot access back of empty RingBuffer");
            return pm_Buffer[(pm_Head + pm_MaxCapacity - 1) % pm_MaxCapacity];
        }

        [[nodiscard]] constexpr const T& Back() const
        {
            assert(not IsEmpty() and "Cannot access back of empty RingBuffer");
            return pm_Buffer[(pm_Head + pm_MaxCapacity - 1) % pm_MaxCapacity];
        }

        [[nodiscard]] constexpr T& At(size_t index)
        {
            assert(index < pm_Size and "Index out of bounds");
            return pm_Buffer[(pm_Tail + index) % pm_MaxCapacity];
        }

        [[nodiscard]] constexpr const T& At(size_t index) const
        {
            assert(index < pm_Size and "Index out of bounds");
            return pm_Buffer[(pm_Tail + index) % pm_MaxCapacity];
        }

        [[nodiscard]] constexpr size_t CurrentSize() const noexcept
        {
            return pm_Size;
        }

        [[nodiscard]] constexpr size_t MaxCapacity() const noexcept
        {
            return pm_MaxCapacity;
        }

        [[nodiscard]] constexpr bool IsEmpty() const noexcept
        {
            return pm_Size == 0;
        }

        [[nodiscard]] constexpr bool IsFull() const noexcept
        {
            return pm_Size == pm_MaxCapacity;
        }

    private:
        array<T, pm_MaxCapacity> pm_Buffer{};
        size_t pm_Head = 0;
        size_t pm_Tail = 0;
        size_t pm_Size = 0;

    private:
        constexpr void Push(const T& value)
        {
            pm_Buffer[pm_Head] = value;
            pm_Head = (pm_Head + 1) % pm_MaxCapacity;
            ++pm_Size;
        }

        constexpr void Push(T&& value)
        {
            pm_Buffer[pm_Head] = move(value);
            pm_Head = (pm_Head + 1) % pm_MaxCapacity;
            ++pm_Size;
        }
    };

    template<typename T, size_t pm_MaxCapacity>
    struct FixedPushBuffer
    {
        constexpr void PushBack(const T& fp_Item)
        {
            assert(pm_CurrentIndex < pm_MaxCapacity and "FixedPushBuffer overflowed!");
            pm_Data[pm_CurrentIndex++] = fp_Item;
        }

        [[nodiscard]] constexpr T& operator[](size_t fp_Index)
        {
            assert(fp_Index < pm_CurrentIndex);
            return pm_Data[fp_Index];
        }

        [[nodiscard]] constexpr const T& operator[](size_t fp_Index) const
        {
            assert(fp_Index < pm_CurrentIndex);
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

namespace GGPO {

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
        string Log;
        uint8_t Level = 69;
    };

    //////////////////////////////////////////////
    // Logger Class
    //////////////////////////////////////////////

    class Logger
    {
    //////////////////////////////////////////////
    // Private Destructor
    //////////////////////////////////////////////
    private:
        ~Logger()
        {
            pm_LogSnapshotBuffer.reset();
        }

    //////////////////////////////////////////////
    // Public Constructor
    //////////////////////////////////////////////
    public:
        Logger() = default;
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

    ////////////////////////////////////////////////
    // Helper Enum For LogLevel Specification
    ////////////////////////////////////////////////
    public:
        enum LogLevel : uint8_t
        {
            Trace = 1 << 0,
            Debug = 1 << 1,
            Info = 1 << 2,
            Warning = 1 << 3,
            Error = 1 << 4,
            Fatal = 1 << 5,
            All = Trace | Debug | Info | Warning | Error | Fatal
        };

    //////////////////////////////////////////////
    // Protected Class Members
    //////////////////////////////////////////////
    protected:
        bool pm_HasBeenInitialized = false;

        unique_ptr<RingBuffer<LogMessage, MAX_NUMBER_OF_LOGS>> pm_LogSnapshotBuffer = nullptr;

        string pm_LoggerName = "No_Logger_Name";

        thread::id pm_ThreadOwnerID;

        uint8_t pm_ActiveLogMask = LogLevel::All;

    //////////////////////////////////////////////
    // Public Methods
    //////////////////////////////////////////////
    public:
        bool
            Initialize
            (
                const string& fp_DesiredLoggerName,
                const uint8_t fp_LogLevelFlags
            )
        {
            if (pm_HasBeenInitialized) //stops accidental reinitialization of logmanager
            {
                PrintError("Logger has already been initialized, Logger is only allowed to initialize once per run");
                return false;
            }

            pm_LogSnapshotBuffer = make_unique<RingBuffer<LogMessage, MAX_NUMBER_OF_LOGS>>();
            pm_ActiveLogMask = fp_LogLevelFlags;
            pm_LoggerName = fp_DesiredLoggerName;
            pm_ThreadOwnerID = this_thread::get_id();

            pm_HasBeenInitialized = true; //well if everything went as planned we should be good to set this to true uwu

            return true;
        }

        //////////////////// Get Last Error ////////////////////

        void
            GetLastError()
        {
            for (int lv_Index = 0; lv_Index < pm_LogSnapshotBuffer->CurrentSize(); lv_Index++)
            {
                //XXX:TODO find last error here uwu >w<
            }
        }

        //////////////////// Logging Functions  ////////////////////

        void
            Log
            (
                const string& fp_Message,
                const string& fp_Sender,
                const uint8_t fp_LogLevel
            )
        {
            //return early without logging if loglevel isnt active or hasnt been initialized or if accessed from the wrong thread
            #ifdef GGPO_DEBUG
                if (not (pm_ActiveLogMask & fp_LogLevel or pm_HasBeenInitialized or AssertThreadAccess("Log"))) return;
            #else
                if (not (pm_ActiveLogMask & fp_LogLevel or pm_HasBeenInitialized)) return;
            #endif

            string f_LogLevel;

            switch (fp_LogLevel)
            {
            case LogLevel::Trace:
                f_LogLevel = "trace";
                break;
            case LogLevel::Debug:
                f_LogLevel = "debug";
                break;
            case LogLevel::Info:
                f_LogLevel = "info";
                break;
            case LogLevel::Warning:
                f_LogLevel = "warn";
                break;
            case LogLevel::Error:
                f_LogLevel = "error"; //not bright oooo soo dark and moody and complex and hard to reach and engage with ><
                break;
            case LogLevel::Fatal:
                f_LogLevel = "fatal";
                break;
            default:
                PrintError(Log("Did not input a valid option for log level in Log()", pm_LoggerName, "error", fp_LogLevel));
                Print(Log(fp_Message, fp_Sender, "error", fp_LogLevel));
                return;
            }

            string f_TimeStamp = GetCurrentTimestamp();
            string f_LogEntry = "[" + f_TimeStamp + "]" + "[" + f_LogLevel + "]" + "[" + fp_Sender + "]: " + fp_Message + "\n";

            pm_LogSnapshotBuffer->ForceEmplace(f_LogEntry, fp_LogLevel); //XXX: adds log to ring buffer in a destructive manner if it wraps around
        }

        void
            LogAndPrint
            (
                const string& fp_Message,
                const string& fp_Sender,
                const uint8_t fp_LogLevel
            )
        {
            //return early without logging if loglevel isnt active or hasnt been initialized or if accessed from the wrong thread
            #ifdef GGPO_DEBUG
                if (not (pm_ActiveLogMask & fp_LogLevel or pm_HasBeenInitialized or AssertThreadAccess("Log"))) return;
            #else
                if (not (pm_ActiveLogMask & fp_LogLevel or pm_HasBeenInitialized)) return;
            #endif

            switch(fp_LogLevel)
            {
                case LogLevel::Trace:
                    Print(Log(fp_Message, fp_Sender, "trace", fp_LogLevel), Colours::BrightWhite);
                    break;
                case LogLevel::Debug:
                    Print(Log(fp_Message, fp_Sender, "debug", fp_LogLevel), Colours::BrightBlue);
                    break;
                case LogLevel::Info:
                    Print(Log(fp_Message, fp_Sender, "info", fp_LogLevel), Colours::BrightGreen);
                    break;
                case LogLevel::Warning:
                    Print(Log(fp_Message, fp_Sender, "warn", fp_LogLevel), Colours::BrightYellow);
                    break;
                case LogLevel::Error:
                    PrintError(Log(fp_Message, fp_Sender, "error", fp_LogLevel), Colours::Red); //not bright oooo soo dark and moody and complex and hard to reach and engage with ><
                    break;
                case LogLevel::Fatal:
                    PrintError(Log(fp_Message, fp_Sender, "fatal", fp_LogLevel), Colours::BrightMagenta);
                    break;
                default:
                    PrintError(Log("Did not input a valid option for log level in LogAndPrint()", "Logger", "error", fp_LogLevel));
                    Print(Log(fp_Message, fp_Sender, "error", fp_LogLevel));
                
            }
        }

    //////////////////////////////////////////////
    // Protected Methods
    //////////////////////////////////////////////
    protected:
        //////////////////// Protected Logging Function  ////////////////////
        //XXX: this is protected since it's supposed to be only called from LogAndPrint()
        string
            Log
            (
                const string& fp_Message,
                const string& fp_Sender,
                const string& fp_LogLevel,
                const uint8_t fp_IntLevel
            )
        {
            const string f_TimeStamp = GetCurrentTimestamp();
            const string f_LogEntry = "[" + f_TimeStamp + "]" + "[" + fp_LogLevel + "]" + "[" + fp_Sender + "]: " + fp_Message;

            pm_LogSnapshotBuffer->ForceEmplace(f_LogEntry, fp_IntLevel); //XXX: adds log to ring buffer in a destructive manner if it wraps around

            return f_LogEntry;
        }

        inline string //thank you chat-gpt uwu
            GetCurrentTimestamp()
            const noexcept
        {
            auto now = chrono::system_clock::now();
            auto time_t_now = chrono::system_clock::to_time_t(now);

            tm local_time{};

            #if defined(_WIN32) || defined(_WIN64) //needa do this since localtime() isnt threadsafe uwu
                localtime_s(&local_time, &time_t_now);
            #else
                localtime_r(&time_t_now, &local_time);
            #endif

            stringstream ss;
            ss << put_time(&local_time, "%Y-%m-%d %H:%M:%S");

            auto since_epoch = now.time_since_epoch();
            auto milliseconds = chrono::duration_cast<chrono::milliseconds>(since_epoch).count() % 1000;

            ss << '.' << setfill('0') << setw(3) << milliseconds;

            return ss.str();
        }

        #ifdef GGPO_DEBUG
            inline bool ///XXX: used for testing, this method should never call exit() for a production release, since all logging is hidden away from the game engine dev
                AssertThreadAccess(const string& fp_FunctionName) //we don't require a lock since this method guarantees only one thread is operating on any data within the LogManager instance
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
        #endif
    };
} //gonna implement namespace after i compile ig ill add that to the TODO

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Socket Macro Defintions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace GGPO
{
#if defined(_WIN32) || defined(_WIN64)
#include <winsock2.h>
#include <ws2tcpip.h>

    typedef SOCKET GGPO_SOCKET;
#define GGPO_INVALID_SOCKET (SOCKET)(~0)

#define GGPO_GET_LAST_ERROR() WSAGetLastError()
#define GGPO_CLOSE_SOCKET(__arg) closesocket(__arg)
#define GGPO_SOCKET_ERROR_CODE WSAEWOULDBLOCK
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


    typedef uint64_t GGPO_SOCKET;
#define GGPO_INVALID_SOCKET (-1)

#define GGPO_GET_LAST_ERROR() errno
#define GGPO_CLOSE_SOCKET(__arg) close(__arg)
#define GGPO_SOCKET_ERROR_CODE EWOULDBLOCK
#endif

    constexpr auto GGPO_SOCKET_ERROR(-1);
    constexpr auto MAX_UDP_ENDPOINTS(16);

    static const int MAX_UDP_PACKET_SIZE = 4096;

    static GGPO_SOCKET
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
        setsockopt(f_Socket, SOL_SOCKET, SO_DONTLINGER, (const char*)&optval, sizeof optval);

        // non-blocking...
        #if defined(_WIN32)
            u_long iMode = 1;
            ioctlsocket(f_Socket, FIONBIO, &iMode);
        #else
            int flags = fcntl(s, F_GETFL, 0);
            fcntl(s, F_SETFL, flags | O_NONBLOCK);
        #endif

        f_SocketIn.sin_family = AF_INET;
        f_SocketIn.sin_addr.s_addr = htonl(INADDR_ANY);

        for (port = bind_port; port <= bind_port + retries; port++)
        {
            f_SocketIn.sin_port = htons(port);

            if (bind(f_Socket, (sockaddr*)&f_SocketIn, sizeof f_SocketIn) != GGPO_SOCKET_ERROR)
            {
                logger->GGPO_LOG(format("Udp bound to port: {}.", port), "udp.cpp", Logger::LogLevel::Info);
                return f_Socket;
            }
        }

        GGPO_CLOSE_SOCKET(f_Socket);

        return GGPO_INVALID_SOCKET;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

constexpr int MAX_PLAYERS = 4;
constexpr int MAX_PREDICTION_FRAMES = 8;
constexpr int MAX_SPECTATORS = 32;

constexpr int SPECTATOR_INPUT_INTERVAL = 4;

namespace GGPO
{
    typedef int PlayerHandle;

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
        FATAL_DESYNC = 12
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
        default: return "Unknown  error.";
        }
    }

    constexpr bool
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

    consteval string_view
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace GGPO{

#ifndef GGPO_MAX_INT
#  define GGPO_MAX_INT          0xEFFFFFF
#endif

#ifndef GGPO_MAX
#  define GGPO_MAX(x, y)        (((x) > (y)) ? (x) : (y))
#endif

#ifndef GGPO_MIN
#  define GGPO_MIN(x, y)        (((x) < (y)) ? (x) : (y))
#endif

#define GGPO_ASSERT(x)                                           
     //do {                                                     
     //   if (!(x)) {                                           
     //      //char assert_buf[1024];                             
     //      snprintf(assert_buf, sizeof(assert_buf) - 1, "Assertion: %s @ %s:%d (pid:%d)", #x, __FILE__, __LINE__, Platform::GetProcessID()); \
       //      //Logger::GGPO_LOGGER().LogAndPrint(to_string(assert_buf), "", "");                           
       //      Platform::AssertFailed(assert_buf);                
       //      exit(0);                                           
       //   }                                                     
       //} while (false)

#define GGPO_ARRAY_SIZE(x) sizeof(x) / sizeof(x[0])

// Platform-Specific Includes
#if defined(_WIN32) || defined(_WIN64)

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>

#include <cstdint>

#include <timeapi.h> //apparently i need this idk where i took it out but i did tho lmfao

    class Platform
    {
    public:  // types
        typedef DWORD ProcessID;

    public:  // functions
        static ProcessID GetProcessID()
        {
            return GetCurrentProcessId();
        }

        static void AssertFailed(char* msg)
        {
            MessageBoxA(NULL, msg, "GGPO Assertion Failed", MB_OK | MB_ICONEXCLAMATION);
        }

        static uint32_t GetCurrentTimeMS()
        {
            return timeGetTime();
        }

        static int GetConfigInt(const char* name);
        static bool GetConfigBool(const char* name);
    };

#elif defined(__linux__) or defined(__APPLE__)

#include <time.h>
#include <cstdint>

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

    class Platform
    {
    public:  // types
        typedef pid_t ProcessID;

    public:  // functions
        static ProcessID GetProcessID()
        {
            return getpid();
        }

        static void AssertFailed(const char* msg) {} //idek ill figure out whether i like the MessageBoxA thing from windows i feel like regular assert is fine but whatever
        static uint32_t GetCurrentTimeMS();
    };

#else

#error Unsupported platform!

#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace GGPO
{
#if defined(_WIN32) || defined(_WIN64)

    int
        Platform::GetConfigInt(const char* name)
    {
        char buf[1024];

        if (GetEnvironmentVariable(name, buf, GGPO_ARRAY_SIZE(buf)) == 0)
        {
            return 0;
        }

        return atoi(buf);
    }

    bool
        Platform::GetConfigBool(const char* name)
    {
        char buf[1024];

        if (GetEnvironmentVariable(name, buf, GGPO_ARRAY_SIZE(buf)) == 0)
        {
            return false;
        }

        return atoi(buf) != 0 || _stricmp(buf, "true") == 0;
    }

#elif defined(__linux__) || defined(__APPLE__)

    struct timespec start = { 0 };

    uint32_t
        Platform::GetCurrentTimeMS()
    {
        if (start.tv_sec == 0 && start.tv_nsec == 0)
        {
            clock_gettime(CLOCK_MONOTONIC, &start);
            return 0;
        }

        struct timespec current;

        clock_gettime(CLOCK_MONOTONIC, &current);

        return ((current.tv_sec - start.tv_sec) * 1000) +
            ((current.tv_nsec - start.tv_nsec) / 1000000); //WEIRD: THIS HAD A TRAILING + AND IDK Y WTF remember dayt
    }

#endif //Unix OS Check //Windows OS Check
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// GAMEINPUT_MAX_BYTES * GAMEINPUT_MAX_PLAYERS * 8 must be less than
    // 2^BITVECTOR_NIBBLE_SIZE (see bitvector.h)

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
                logger->GGPO_LOG(format("frames don't match: {}, {}", frame, other.frame), "game_input.cpp", Logger::LogLevel::Info);
            }
            if (size != other.size)
            {
                logger->GGPO_LOG(format("sizes don't match: {}, {}", size, other.size), "game_input.cpp", Logger::LogLevel::Info);
            }
            if (memcmp(bits, other.bits, size))
            {
                logger->GGPO_LOG("bits don't match", "game_input.cpp", Logger::LogLevel::Info);
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

#define PREVIOUS_FRAME(offset)   (((offset) == 0) ? (INPUT_QUEUE_LENGTH - 1) : ((offset) - 1))

namespace GGPO
{
    constexpr int INPUT_QUEUE_LENGTH = 128;
    constexpr int DEFAULT_INPUT_SIZE = 4;

    class InputQueue
    {
    public:
        InputQueue(int input_size = DEFAULT_INPUT_SIZE) //???? why do it like this lmfao y not just do it inside the constructor body lol
        {
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
            GetLastConfirmedFrame(Logger* logger)
        {
            logger->GGPO_LOG(format("returning last confirmed frame {}.", _last_added_frame), "input_queue.cpp", Logger::LogLevel::Info);
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
            ResetPrediction(int frame)
        {
            GGPO_ASSERT(_first_incorrect_frame == GameInput::NullFrame || frame <= _first_incorrect_frame);

            logger->LogAndPrint(format("resetting all prediction errors back to frame {}.", frame), "input_queue.cpp", Logger::LogLevel::Info);

            /*
             * There's nothing really to do other than reset our prediction
             * state and the incorrect frame counter...
             */
            _prediction.frame = GameInput::NullFrame;
            _first_incorrect_frame = GameInput::NullFrame;
            _last_frame_requested = GameInput::NullFrame;
        }

        void
            DiscardConfirmedFrames(int frame)
        {
            GGPO_ASSERT(frame >= 0);

            if (_last_frame_requested != GameInput::NullFrame)
            {
                frame = GGPO_MIN(frame, _last_frame_requested);
            }

            logger->LogAndPrint(format("discarding confirmed frames up to {} (last_added:{} length:{} [head:{} tail:{}]).", frame, _last_added_frame, _length, _head, _tail), "input_queue.cpp", Logger::LogLevel::Info);

            if (frame >= _last_added_frame)
            {
                _tail = _head;
            }
            else
            {
                int offset = frame - _inputs[_tail].frame + 1;

                logger->LogAndPrint(format("difference of {} frames.", offset), "input_queue.cpp", Logger::LogLevel::Info);
                GGPO_ASSERT(offset >= 0);

                _tail = (_tail + offset) % INPUT_QUEUE_LENGTH;
                _length -= offset;
            }

            logger->LogAndPrint(format("after discarding, new tail is {} (frame:{}).", _tail, _inputs[_tail].frame), "input_queue.cpp", Logger::LogLevel::Info);
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
            logger->LogAndPrint(format("requesting input frame {}.", requested_frame), "input_queue.cpp", Logger::LogLevel::Info);

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
                    logger->LogAndPrint(format("returning confirmed frame number {}.", input->frame), "input_queue.cpp", Logger::LogLevel::Info);
                    return true;
                }

                /*
                * The requested frame isn't in the queue.  Bummer.  This means we need
                * to return a prediction frame.  Predict that the user will do the
                * same thing they did last time.
                */
                if (requested_frame == 0)
                {
                    logger->LogAndPrint("basing new prediction frame from nothing, you're client wants frame 0.", "input_queue.cpp", Logger::LogLevel::Info);
                    _prediction.erase();
                }
                else if (_last_added_frame == GameInput::NullFrame)
                {
                    logger->LogAndPrint("basing new prediction frame from nothing, since we have no frames yet.", "input_queue.cpp", Logger::LogLevel::Info);
                    _prediction.erase();
                }
                else
                {
                    logger->LogAndPrint(format("basing new prediction frame from previously added frame (queue entry:{}, frame:{}).", PREVIOUS_FRAME(_head), _inputs[PREVIOUS_FRAME(_head)].frame), "input_queue.cpp", Logger::LogLevel::Info);
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
            logger->LogAndPrint(format("returning prediction frame number {} ({}).", input->frame, _prediction.frame), "input_queue.cpp", Logger::LogLevel::Info);

            return false;
        }

        void
            AddInput(GameInput& input)
        {
            int new_frame;

            logger->LogAndPrint(format("adding input frame number {} to queue.", input.frame), "input_queue.cpp", Logger::LogLevel::Info);

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
            logger->LogAndPrint(format("advancing queue head to frame {}.", frame), "input_queue.cpp", Logger::LogLevel::Info);

            int expected_frame = _first_frame ? 0 : _inputs[PREVIOUS_FRAME(_head)].frame + 1;

            frame += _frame_delay;

            if (expected_frame > frame)
            {
                /*
                * This can occur when the frame delay has dropped since the last
                * time we shoved a frame into the system.  In this case, there's
                * no room on the queue.  Toss it.
                */
                logger->LogAndPrint(format("Dropping input frame {} (expected next frame to be {}).", frame, expected_frame), "input_queue.cpp", Logger::LogLevel::Info);
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
                logger->LogAndPrint(format("Adding padding frame {} to account for change in frame delay.", expected_frame), "input_queue.cpp", Logger::LogLevel::Info);
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
            logger->LogAndPrint(format("adding delayed input frame number {} to queue.", frame_number), "input_queue.cpp", Logger::LogLevel::Info);

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
                    logger->LogAndPrint(format("frame {} does not match prediction.  marking error.", frame_number), "input_queue.cpp", Logger::LogLevel::Info);
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
                    logger->LogAndPrint("prediction is correct!  dumping out of prediction mode.", "input_queue.cpp", Logger::LogLevel::Info);
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
              recommend_frame_wait_duration(bool require_idle_input)
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

              static int count = 0;
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

              logger->LogAndPrint(format("iteration {}:  sleep frames is {}", count, sleep_frames), "timesync.cpp", Logger::LogLevel::Info);

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
                          logger->LogAndPrint(format("iteration {}:  rejecting due to input stuff at position {}...!!!", count, i), "timesync.cpp", Logger::LogLevel::Info);
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
 
 /* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */



namespace GGPO
{
     class SyncTestBackend;

     class Sync
     {
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

             for (int i = 0; i < GGPO_ARRAY_SIZE(_savedstate.frames); i++)
             {
                 _callbacks.free_buffer((void*)&_savedstate.frames[i].buf); //lmfao this is ridiculous
             }

             delete[] _input_queues;
             _input_queues = NULL;
         }

         void
             Init(Sync::Config& config)
         {
             _config = config;
             _framecount = 0;
             _rollingback = false;

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
                 logger->LogAndPrint("Rejecting input from emulator: reached prediction barrier.", "sync.cpp", Logger::LogLevel::Info);
                 return false;
             }

             if (_framecount == 0)
             {
                 SaveCurrentFrame();
             }

             logger->LogAndPrint(format("Sending undelayed local frame {} to queue {}.", _framecount, queue), "sync.cpp", Logger::LogLevel::Info);

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
             CheckSimulation(int timeout)
         {
             int seek_to;
             if (not CheckSimulationConsistency(&seek_to))
             {
                 AdjustSimulation(seek_to);
             }
         }

         void
             AdjustSimulation(int seek_to)
         {
             int framecount = _framecount;
             int count = _framecount - seek_to;

             logger->LogAndPrint("Catching up", "sync.cpp", Logger::LogLevel::Info);
             _rollingback = true;

             /*
              * Flush our input queue and load the last frame.
              */
             LoadFrame(seek_to);
             GGPO_ASSERT(_framecount == seek_to);

             /*
              * Advance frame by frame (stuffing notifications back to
              * the master).
              */
             ResetPrediction(_framecount);

             for (int i = 0; i < count; i++)
             {
                 _callbacks.advance_frame(0);
             }

             GGPO_ASSERT(_framecount == framecount);

             _rollingback = false;

             logger->LogAndPrint("---", "sync.cpp", Logger::LogLevel::Info); //?????????????????????????
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
             InRollback()
             const
         {
             return _rollingback;
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
             // find the frame in question
             if (frame == _framecount)
             {
                 logger->LogAndPrint("Skipping NOP.", "sync.cpp", Logger::LogLevel::Info);
                 return;
             }

             // Move the head pointer back and load it up
             _savedstate.head = FindSavedFrameIndex(frame);
             SavedFrame* state = _savedstate.frames + _savedstate.head;

             logger->LogAndPrint(format("=== Loading frame info {} (checksum: {}).", state->frame, state->checksum), "sync.cpp", Logger::LogLevel::Info);

             GGPO_ASSERT(state->buf and state->cbuf);

             _callbacks.load_game_state(state->buf);

             // Reset framecount and the head of the state ring-buffer to point in
             // advance of the current frame (as if we had just finished executing it).
             _framecount = state->frame;
             _savedstate.head = (_savedstate.head + 1) % GGPO_ARRAY_SIZE(_savedstate.frames);
         }

         void
             SaveCurrentFrame()
         {
             /*
              * See StateCompress for the real save feature implemented by FinalBurn.
              * Write everything into the head, then advance the head pointer.
              */
             SavedFrame* state = _savedstate.frames + _savedstate.head;

             state->frame = _framecount;
             _callbacks.save_game_state(state->buf, &state->frame, &state->checksum, state->frame);

             logger->LogAndPrint(format("=== Saved frame info {} (checksum: {}).", state->frame, state->checksum), "sync.cpp", Logger::LogLevel::Info);
             _savedstate.head = (_savedstate.head + 1) % GGPO_ARRAY_SIZE(_savedstate.frames);
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

         bool
             CheckSimulationConsistency(int* seekTo)
         {
             int first_incorrect = GameInput::NullFrame;

             for (int i = 0; i < _config.num_players; i++)
             {
                 int incorrect = _input_queues[i].GetFirstIncorrectFrame();
                 logger->LogAndPrint(format("considering incorrect frame {} reported by queue {}.", incorrect, i), "sync.cpp", Logger::LogLevel::Info);

                 if (incorrect != GameInput::NullFrame and (first_incorrect == GameInput::NullFrame or incorrect < first_incorrect))
                 {
                     first_incorrect = incorrect;
                 }
             }

             if (first_incorrect == GameInput::NullFrame)
             {
                 logger->LogAndPrint("prediction ok.  proceeding.", "sync.cpp", Logger::LogLevel::Info);
                 return true;
             }

             *seekTo = first_incorrect;

             return false;
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

         bool           _rollingback;
         int            _last_confirmed_frame;
         int            _framecount;
         int            _max_prediction_frames;

         InputQueue* _input_queues;

         RingBuffer<Event, 32> _event_queue;
         UdpMsg::connect_status* _local_connect_status;
     };

}

/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

 
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
              _handles[_handle_count++] = CreateEvent(NULL, true, false, NULL);
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
                  _start_time = Platform::GetCurrentTimeMS();
              }

              int elapsed = Platform::GetCurrentTimeMS() - _start_time;
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

////////////////////////////////////////////////////////////////////////////////// ggponet.h
 
 


 /* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */
 
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
             udp_logger = make_unique<Logger>();
             //udp_logger->Initialize()
         }

         void
             Init(uint16_t port, Poll* poll, Callbacks* callbacks)
         {
             _callbacks = callbacks;

             _poll = poll;
             _poll->RegisterLoop(this);

             logger->LogAndPrint(format("binding udp socket to port {}.", port), "udp.cpp", Logger::LogLevel::Info);
             _socket = CreateSocket(port, 0);
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
                 DWORD err = GGPO_GET_LAST_ERROR();
                 logger->LogAndPrint(format("unknown error in sendto (erro: {}  wsaerr: {}).", res, err), "udp.cpp", Logger::LogLevel::Error);
                 GGPO_ASSERT(FALSE && "Unknown error in sendto");
             }

             char dst_ip[1024];

             logger->LogAndPrint(format("sent packet length {} to {}:{} (ret:{}).", len, inet_ntop(AF_INET, (void*)&to->sin_addr, dst_ip, GGPO_ARRAY_SIZE(dst_ip)), ntohs(to->sin_port), res), "udp.cpp", Logger::LogLevel::Error);
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
                         logger->LogAndPrint(format("recvfrom GGPO_GET_LAST_ERROR returned {} ({}).", error, error), "udp.cpp", Logger::LogLevel::Error);
                     }

                     break;
                 }
                 else if (len > 0)
                 {
                     char src_ip[1024];
                     logger->LogAndPrint(format("recvfrom returned (len:{}  from:{}:{}).", len, inet_ntop(AF_INET, (void*)&recv_addr.sin_addr, src_ip, GGPO_ARRAY_SIZE(src_ip)), ntohs(recv_addr.sin_port)), "udp.cpp", Logger::LogLevel::Error);
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
 
 /* -----------------------------------------------------------------------
 * .net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

static const int UDP_HEADER_SIZE = 28;     /* Size of IP + UDP headers */
static const int NUM_SYNC_PACKETS = 5;
static const int SYNC_RETRY_INTERVAL = 2000;
static const int SYNC_FIRST_RETRY_INTERVAL = 500;
static const int RUNNING_RETRY_INTERVAL = 200;
static const int KEEP_ALIVE_INTERVAL = 200;
static const int QUALITY_REPORT_INTERVAL = 1000;
static const int NETWORK_STATS_INTERVAL = 1000;
static const int UDP_SHUTDOWN_TIMER = 5000;
static const int MAX_SEQ_DISTANCE = (1 << 15);

namespace GGPO
{
    class UdpProtocol
    {
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

            unsigned int now = Platform::GetCurrentTimeMS();
            unsigned int next_interval;

            PumpSendQueue();

            switch (_current_state)
            {
            case Syncing:
                next_interval = (_state.sync.roundtrips_remaining == NUM_SYNC_PACKETS) ? SYNC_FIRST_RETRY_INTERVAL : SYNC_RETRY_INTERVAL;
                if (_last_send_time and _last_send_time + next_interval < now)
                {
                    logger->LogAndPrint(format("No luck syncing after {} ms... Re-queueing sync packet.", next_interval), "udp_proto.cpp", Logger::LogLevel::Info);
                    SendSyncRequest();
                }
                break;

            case Running:
                // xxx: rig all this up with a timer wrapper
                if (not _state.running.last_input_packet_recv_time or _state.running.last_input_packet_recv_time + RUNNING_RETRY_INTERVAL < now)
                {
                    logger->LogAndPrint(format("Haven't exchanged packets in a while (last received:{}  last sent:{}).  Resending.", _last_received_input.frame, _last_sent_input.frame), "udp_proto.cpp", Logger::LogLevel::Info);
                    SendPendingOutput();
                    _state.running.last_input_packet_recv_time = now;
                }

                if (not _state.running.last_quality_report_time or _state.running.last_quality_report_time + QUALITY_REPORT_INTERVAL < now)
                {
                    UdpMsg* msg = new UdpMsg(UdpMsg::QualityReport);
                    msg->u.quality_report.ping = Platform::GetCurrentTimeMS();
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
                    logger->LogAndPrint("Sending keep alive packet", "udp_proto.cpp", Logger::LogLevel::Info);
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
                    logger->LogAndPrint(format("Endpoint has stopped receiving packets for {} ms.  Sending notification.", _disconnect_notify_start), "udp_proto.cpp", Logger::LogLevel::Info);
                    Event e(Event::NetworkInterrupted);
                    e.u.network_interrupted.disconnect_timeout = _disconnect_timeout - _disconnect_notify_start;
                    QueueEvent(e);
                    _disconnect_notify_sent = true;
                }

                if (_disconnect_timeout and (_last_recv_time + _disconnect_timeout < now))
                {
                    if (not _disconnect_event_sent)
                    {
                        logger->LogAndPrint(format("Endpoint has stopped receiving packets for {} ms.  Disconnecting.", _disconnect_timeout), "udp_proto.cpp", Logger::LogLevel::Info);
                        QueueEvent(Event(Event::Disconnected));
                        _disconnect_event_sent = true;
                    }
                }
                break;

            case Disconnected:
                if (_shutdown_timeout < now)
                {
                    logger->LogAndPrint("Shutting down udp connection.", "udp_proto.cpp", Logger::LogLevel::Info);
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
                    _pending_output.SafePush(input);
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
                    logger->LogAndPrint(format("dropping out of order packet (seq: {}, last seq:{})", seq, _next_recv_seq), "udp_proto.cpp", Logger::LogLevel::Info);
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
                _last_recv_time = Platform::GetCurrentTimeMS();

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
            _shutdown_timeout = Platform::GetCurrentTimeMS() + UDP_SHUTDOWN_TIMER;
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
            return _timesync.recommend_frame_wait_duration(false);
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
            int         queue_time;
            sockaddr_in dest_addr;
            UdpMsg* msg;

            QueueEntry() {}
            QueueEntry(int time, sockaddr_in& dst, UdpMsg* m) : queue_time(time), dest_addr(dst), msg(m) { }
        };

        void
            UpdateNetworkStats(void)
        {
            int now = Platform::GetCurrentTimeMS();

            if (_stats_start_time == 0)
            {
                _stats_start_time = now;
            }

            int total_bytes_sent = _bytes_sent + (UDP_HEADER_SIZE * _packets_sent);
            float seconds = (float)((now - _stats_start_time) / 1000.0);
            float Bps = total_bytes_sent / seconds;
            float udp_overhead = (float)(100.0 * (UDP_HEADER_SIZE * _packets_sent) / _bytes_sent);

            _kbps_sent = int(Bps / 1024);

            logger->LogAndPrint
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
                "udp_proto.cpp",
                Logger::LogLevel::Info
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
            LogMsg(const char* prefix, UdpMsg* msg)
        {
            switch (msg->hdr.type)
            {
            case UdpMsg::SyncRequest:
                logger->LogAndPrint(format("{} sync-request ({}).", prefix, msg->u.sync_request.random_request), "udp_proto.cpp", Logger::LogLevel::Info);
                break;

            case UdpMsg::SyncReply:
                logger->LogAndPrint(format("{} sync-reply ({}).", prefix, msg->u.sync_reply.random_reply), "udp_proto.cpp", Logger::LogLevel::Info);
                break;

            case UdpMsg::QualityReport:
                logger->LogAndPrint(format("{} quality report.", prefix), "udp_proto.cpp", Logger::LogLevel::Info);
                break;

            case UdpMsg::QualityReply:
                logger->LogAndPrint(format("{} quality reply.", prefix), "udp_proto.cpp", Logger::LogLevel::Info);
                break;

            case UdpMsg::KeepAlive:
                logger->LogAndPrint(format("{} keep alive.", prefix), "udp_proto.cpp", Logger::LogLevel::Info);
                break;

            case UdpMsg::Input:
                logger->LogAndPrint(format("{} game-compressed-input {} (+ {} bits).", prefix, msg->u.input.start_frame, msg->u.input.num_bits), "udp_proto.cpp", Logger::LogLevel::Info);
                break;

            case UdpMsg::InputAck:
                logger->LogAndPrint(format("{} input ack.", prefix), "udp_proto.cpp", Logger::LogLevel::Info);
                break;

            default:
                GGPO_ASSERT(FALSE and "Unknown UdpMsg type.");
            }
        }

        void
            LogEvent(const char* prefix, const UdpProtocol::Event& evt)
        {
            switch (evt.type)
            {
            case UdpProtocol::Event::Synchronzied:
                logger->LogAndPrint(format("{} (event: Synchronzied).", prefix), "udp_proto.cpp", Logger::LogLevel::Info);
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
            _last_send_time = Platform::GetCurrentTimeMS();
            _bytes_sent += msg->PacketSize();

            msg->hdr.magic = _magic_number;
            msg->hdr.sequence_number = _next_send_seq++;

            _send_queue.SafePush(QueueEntry(Platform::GetCurrentTimeMS(), _peer_addr, msg));
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

                    if (Platform::GetCurrentTimeMS() < _send_queue.Front().queue_time + jitter)
                    {
                        break;
                    }
                }
                if (_oop_percent and not _oo_packet.msg and ((rand() % 100) < _oop_percent))
                {
                    int delay = rand() % (_send_latency * 10 + 1000);

                    if (entry.msg) //check for a nullptr dereference
                    {
                        logger->LogAndPrint(format("creating rogue oop (seq: {}  delay: {})", entry.msg->hdr.sequence_number, delay), "udp_proto.cpp", Logger::LogLevel::Info);
                    }
                    else
                    {
                        //???????????????????????????????????????????????????????? WHY IS THIS HERE TONY ANSWER ME
                    }

                    _oo_packet.send_time = Platform::GetCurrentTimeMS() + delay;
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
            if (_oo_packet.msg and _oo_packet.send_time < Platform::GetCurrentTimeMS())
            {
                logger->LogAndPrint("sending rogue oop!", "udp_proto.cpp", Logger::LogLevel::Info);

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
                        GGPO_ASSERT((GAMEINPUT_MAX_BYTES * GAMEINPUT_MAX_PLAYERS * 8) < (1 << BITVECTOR_NIBBLE_SIZE));

                        for (i = 0; i < current.size * 8; i++)
                        {
                            GGPO_ASSERT(i < (1 << BITVECTOR_NIBBLE_SIZE));

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
            GGPO_ASSERT(FALSE and "Invalid msg in UdpProtocol");
            return false;
        }

        bool
            OnSyncRequest(UdpMsg* msg, int len)
        {
            if (_remote_magic_number != 0 and msg->hdr.magic != _remote_magic_number)
            {
                logger->LogAndPrint(format("Ignoring sync request from unknown endpoint ({} != {}).", msg->hdr.magic, _remote_magic_number), "udp_proto.cpp", Logger::LogLevel::Error);
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
                logger->LogAndPrint("Ignoring SyncReply while not synching.", "udp_proto.cpp", Logger::LogLevel::Info);
                return msg->hdr.magic == _remote_magic_number;
            }

            if (msg->u.sync_reply.random_reply != _state.sync.random)
            {
                logger->LogAndPrint(format("sync reply {} != {}.  Keep looking...", msg->u.sync_reply.random_reply, _state.sync.random), "udp_proto.cpp", Logger::LogLevel::Info);
                return false;
            }

            if (not _connected)
            {
                QueueEvent(Event(Event::Connected));
                _connected = true;
            }

            logger->LogAndPrint(format("Checking sync state ({} round trips remaining).", _state.sync.roundtrips_remaining), "udp_proto.cpp", Logger::LogLevel::Info);

            if (--_state.sync.roundtrips_remaining == 0)
            {
                logger->LogAndPrint("Synchronized!", "udp_proto.cpp", Logger::LogLevel::Info);
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
                    logger->LogAndPrint("Disconnecting endpoint on remote request.", "udp_proto.cpp", Logger::LogLevel::Info);
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

                        _last_received_input.desc(desc, GGPO_ARRAY_SIZE(desc));

                        _state.running.last_input_packet_recv_time = Platform::GetCurrentTimeMS();

                        logger->LogAndPrint(format("Sending frame {} to emu queue {} ({}).", _last_received_input.frame, _queue, desc), "udp_proto.cpp", Logger::LogLevel::Info);
                        QueueEvent(evt);

                    }
                    else
                    {
                        logger->LogAndPrint(format("Skipping past frame:({}) current is {}.", currentFrame, _last_received_input.frame), "udp_proto.cpp", Logger::LogLevel::Info);
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
                logger->LogAndPrint(format("Throwing away pending output frame {}", _pending_output.Front().frame), "udp_proto.cpp", Logger::LogLevel::Info);
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
                logger->LogAndPrint(format("Throwing away pending output frame {}", _pending_output.Front().frame), "udp_proto.cpp", Logger::LogLevel::Info);
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
            _round_trip_time = Platform::GetCurrentTimeMS() - msg->u.quality_reply.pong;
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
            int         send_time;
            sockaddr_in dest_addr;
            UdpMsg* msg;
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
 
/* -----------------------------------------------------------------------
 * GGPO.net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

 constexpr int MAX_COMPRESSED_BITS = 4096;
 constexpr int UDP_MSG_MAX_PLAYERS = 4;
 
 //#pragma pack(push, 1)
 
 struct UdpMsg
 {
    enum MsgType 
    {
       Invalid       = 0,
       SyncRequest   = 1,
       SyncReply     = 2,
       Input         = 3,
       QualityReport = 4,
       QualityReply  = 5,
       KeepAlive     = 6,
       InputAck      = 7,
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
 
          int               disconnect_requested:1;
          int               ack_frame:31;
 
          uint16_t            num_bits;
          uint8_t             input_size; // XXX: shouldn't be in every single packet!
          uint8_t             bits[MAX_COMPRESSED_BITS]; /* must be last */
       } input;
 
       struct 
       {
          int               ack_frame:31;
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
              size = (int)((char *)&u.input.bits - (char *)&u.input);
              size += (u.input.num_bits + 7) / 8;
              return size;
       }
 
       GGPO_ASSERT(false); // ??????????
 
       return 0;
    }
 
    UdpMsg(MsgType t) { hdr.type = (uint8_t)t; }
 };
 
 //#pragma pack(pop)


 /* -----------------------------------------------------------------------
 * .net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

namespace GGPO
{
     class SyncTestBackend
     {
     public:
         SyncTestBackend
         (
             string gamename,
             const int frames,
             const int num_players
         ) :
             _sync(NULL)
         {
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

             logger->LogAndPrint(format("End of frame({})...", _sync.GetFrameCount()), "synctest.cpp", Logger::LogLevel::Info);

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
                         logger->LogAndPrint(format("Frame number {} does not match saved frame number {}", info.frame, frame), "synctest.cpp", Logger::LogLevel::Error);
                         logger->LogAndPrint(format("Program will now exit with error: {}", ErrorToString(ErrorCode::FATAL_DESYNC)), "synctest.cpp", Logger::LogLevel::Error);
                         exit(static_cast<int>(ErrorCode::FATAL_DESYNC)); //RAISESYNC ERRROR WAS HERE
                     }

                     int checksum = _sync.GetLastSavedFrame().checksum;

                     if (info.checksum != checksum)
                     {
                         logger->LogAndPrint(format("Checksum for frame {} does not match saved ({} != {})", frame, checksum, info.checksum), "synctest.cpp", Logger::LogLevel::Error);
                         logger->LogAndPrint(format("Program will now exit with error: {}", ErrorToString(ErrorCode::FATAL_DESYNC)), "synctest.cpp", Logger::LogLevel::Error);
                         exit(static_cast<int>(ErrorCode::FATAL_DESYNC)); //RAISESYNC ERRROR WAS HERE
                     }

                     logger->LogAndPrint(format("Checksum {} for frame {} matches.", checksum, info.frame), "synctest.cpp", Logger::LogLevel::Info);
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
             logger->LogAndPrint(format("state-{}-original and {}", _sync.GetFrameCount(), info.buf), "synctest.cpp", Logger::LogLevel::Info);

             logger->LogAndPrint(format("state-{}-replay and {}", _sync.GetFrameCount(), _sync.GetLastSavedFrame().buf), "synctest.cpp", Logger::LogLevel::Info);
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
 
/* -----------------------------------------------------------------------
 * .net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */
 
 constexpr int SPECTATOR_FRAME_BUFFER_SIZE = 64;
 
 namespace GGPO
 {
      class SpectatorBackend
      {
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

          virtual ~SpectatorBackend() = default;
 
 
      public:
          virtual ErrorCode
              DoPoll(int timeout)
          {
              _poll.Pump(0);

              PollUdpProtocolEvents();
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

          virtual ErrorCode
              IncrementFrame(void)
          {
              logger->LogAndPrint(format("End of frame ({})...", _next_input_to_send - 1), "spectator.cpp", Logger::LogLevel::Info);
              DoPoll(0);
              PollUdpProtocolEvents();

              return ErrorCode::OK;
          }
 
          virtual ErrorCode AddPlayer(Player* player, PlayerHandle* handle)
          {
              return ErrorCode::UNSUPPORTED;
          }
 
          virtual ErrorCode AddLocalInput(PlayerHandle player, void* values, int size) //??????
          {
              return ErrorCode::OK;
          }
 
          virtual ErrorCode DisconnectPlayer(PlayerHandle handle)
          {
              return ErrorCode::UNSUPPORTED;
          }
 
          virtual ErrorCode GetNetworkStats(NetworkStats* stats, PlayerHandle handle)
          {
              return ErrorCode::UNSUPPORTED;
          }
 
          virtual ErrorCode SetFrameDelay(PlayerHandle player, int delay)
          {
              return ErrorCode::UNSUPPORTED;
          }
 
          virtual ErrorCode SetDisconnectTimeout(int timeout)
          {
              return ErrorCode::UNSUPPORTED;
          }
 
          virtual ErrorCode SetDisconnectNotifyStart(int timeout)
          {
              return ErrorCode::UNSUPPORTED;
          }
 
      public:
          virtual void
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
          void
              PollUdpProtocolEvents(void)
          {
              UdpProtocol::Event evt;

              while (_host.GetEvent(evt))
              {
                  OnUdpProtocolEvent(evt);
              }
          }
 
          void
              OnUdpProtocolEvent(UdpProtocol::Event& evt)
          {
              Event info;

              switch (evt.type)
              {
              case UdpProtocol::Event::Connected:
                  info.code = EventCode::ConnectedToPeer;
                  info.u.connected.player = 0;
                  _callbacks.on_event(&info);
                  break;

              case UdpProtocol::Event::Synchronizing:
                  info.code = EventCode::SynchronizingWithPeer;
                  info.u.synchronizing.player = 0;
                  info.u.synchronizing.count = evt.u.synchronizing.count;
                  info.u.synchronizing.total = evt.u.synchronizing.total;
                  _callbacks.on_event(&info);
                  break;

              case UdpProtocol::Event::Synchronzied:
                  if (_synchronizing)
                  {
                      info.code = EventCode::SynchronizedWithPeer;
                      info.u.synchronized.player = 0;
                      _callbacks.on_event(&info);

                      info.code = EventCode::Running;
                      _callbacks.on_event(&info);
                      _synchronizing = false;
                  }
                  break;

              case UdpProtocol::Event::NetworkInterrupted:
                  info.code = EventCode::ConnectionInterrupted;
                  info.u.connection_interrupted.player = 0;
                  info.u.connection_interrupted.disconnect_timeout = evt.u.network_interrupted.disconnect_timeout;
                  _callbacks.on_event(&info);
                  break;

              case UdpProtocol::Event::NetworkResumed:
                  info.code = EventCode::ConnectionResumed;
                  info.u.connection_resumed.player = 0;
                  _callbacks.on_event(&info);
                  break;

              case UdpProtocol::Event::Disconnected:
                  info.code = EventCode::DisconnectedFromPeer;
                  info.u.disconnected.player = 0;
                  _callbacks.on_event(&info);
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

 /* -----------------------------------------------------------------------
 * .net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

namespace GGPO
{
     struct Session
     {
         Session() = default;
         ~Session() = default;

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
         unique_ptr<Logger> logger = nullptr;
         RingBuffer<Event, 64> pm_EventQueue;

         Sync pm_Sync;
         Udp pm_Udp;

         vector<UdpProtocol> pm_Endpoints; //needs to be dynamically sized ig for now since total size of lobby isn't knowable from compile time

         Peer2PeerBackend* pm_P2P = nullptr;
         vector<SpectatorBackend> pm_Spectators;
         
     };
}

/* -----------------------------------------------------------------------
 * .net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

static constexpr int RECOMMENDATION_INTERVAL = 240;
static constexpr int DEFAULT_DISCONNECT_TIMEOUT = 5000;
static constexpr int DEFAULT_DISCONNECT_NOTIFY_START = 750;
 
 namespace GGPO
 {
      class Peer2PeerBackend
      {
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
               //_callbacks.begin_game(gamename); //THIS IS DEPRECATED APPARENTLY SO IDK WHAT ITS DOIN HERE
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

                      logger->LogAndPrint(format("last confirmed frame in p2p backend is {}.", total_min_confirmed), "p2p.cpp", Logger::LogLevel::Info);

                      if (total_min_confirmed >= 0)
                      {
                          GGPO_ASSERT(total_min_confirmed != INT_MAX);

                          if (_num_spectators > 0)
                          {
                              while (_next_spectator_frame <= total_min_confirmed)
                              {
                                  logger->LogAndPrint(format("pushing frame {} to spectators.", _next_spectator_frame), "p2p.cpp", Logger::LogLevel::Info);

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
                          logger->LogAndPrint(format("setting confirmed frame in sync to {}.", total_min_confirmed), "p2p.cpp", Logger::LogLevel::Info);

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

                  logger->LogAndPrint(format("setting local connect status for local queue {} to {}", queue, input.frame), "p2p.cpp", Logger::LogLevel::Info);
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

              if (disconnect_flags)
              {
                  *disconnect_flags = flags;
              }

              return ErrorCode::OK;
          }

          virtual ErrorCode
              IncrementFrame(void)
          {
              logger->LogAndPrint(format("End of frame ({})...", _sync.GetFrameCount()), "p2p.cpp", Logger::LogLevel::Info);
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
                  logger->LogAndPrint(format("Disconnecting local player {} at frame {} by user request.", queue, _local_connect_status[queue].last_frame), "p2p.cpp", Logger::LogLevel::Info);
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
                  logger->LogAndPrint(format("Disconnecting queue {} at frame {} by user request.", queue, _local_connect_status[queue].last_frame), "p2p.cpp", Logger::LogLevel::Info);
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
              PlayerHandleToQueue(PlayerHandle player, int* queue)
          {
              int offset = ((int)player - 1);

              if (offset < 0 or offset >= _num_players)
              {
                  return ErrorCode::INVALID_PLAYER_HANDLE;
              }

              *queue = offset;

              return ErrorCode::OK;
          }

          PlayerHandle QueueToPlayerHandle(int queue) { return (PlayerHandle)(queue + 1); }
          PlayerHandle QueueToSpectatorHandle(int queue) { return (PlayerHandle)(queue + 1000); } /* out of range of the player array, basically */
          
          void
              DisconnectPlayerQueue(int queue, int syncto)
          {
              Event info;
              int framecount = _sync.GetFrameCount();

              _endpoints[queue].Disconnect();

              logger->LogAndPrint(format("Changing queue {} local connect status for last frame from {} to {} on disconnect request (current: {}).", queue, _local_connect_status[queue].last_frame, syncto, framecount), "p2p.cpp", Logger::LogLevel::Info);

              _local_connect_status[queue].disconnected = 1;
              _local_connect_status[queue].last_frame = syncto;

              if (syncto < framecount)
              {
                  logger->LogAndPrint(format("adjusting simulation to account for the fact that {} disconnected @ {}.", queue, syncto), "p2p.cpp", Logger::LogLevel::Info);
                  _sync.AdjustSimulation(syncto);
                  logger->LogAndPrint("finished adjusting simulation.", "p2p.cpp", Logger::LogLevel::Info);
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

                  logger->LogAndPrint(format("  local endp: connected = {}, last_received = {}, total_min_confirmed = {}.", not _local_connect_status[i].disconnected, _local_connect_status[i].last_frame, total_min_confirmed), "p2p.cpp", Logger::LogLevel::Info);

                  if (not queue_connected && not _local_connect_status[i].disconnected)
                  {
                      logger->LogAndPrint(format("disconnecting i {} by remote request.", i), "p2p.cpp", Logger::LogLevel::Info);
                      DisconnectPlayerQueue(i, total_min_confirmed);
                  }

                  logger->LogAndPrint(format("  total_min_confirmed = {}.", total_min_confirmed), "p2p.cpp", Logger::LogLevel::Info);
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

                  logger->LogAndPrint(format("considering queue {}.", queue), "p2p.cpp", Logger::LogLevel::Info);

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
                          logger->LogAndPrint(format("  endpoint {}: connected = {}, last_received = {}, queue_min_confirmed = {}.", i, connected, last_received, queue_min_confirmed), "p2p.cpp", Logger::LogLevel::Info);
                      }
                      else
                      {
                          logger->LogAndPrint(format("  endpoint {}: ignoring... not running.", i), "p2p.cpp", Logger::LogLevel::Info);
                      }
                  }
                  // merge in our local status only if we're still connected!
                  if (not _local_connect_status[queue].disconnected)
                  {
                      queue_min_confirmed = GGPO_MIN(_local_connect_status[queue].last_frame, queue_min_confirmed);
                  }

                  logger->LogAndPrint(format("  local endp: connected = {}, last_received = {}, queue_min_confirmed = {}.", not _local_connect_status[queue].disconnected, _local_connect_status[queue].last_frame, queue_min_confirmed), "p2p.cpp", Logger::LogLevel::Info);

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
                          logger->LogAndPrint(format("disconnecting queue {} by remote request.", queue), "p2p.cpp", Logger::LogLevel::Info);
                          DisconnectPlayerQueue(queue, queue_min_confirmed);
                      }
                  }
                  logger->LogAndPrint(format("  total_min_confirmed = {}.", total_min_confirmed), "p2p.cpp", Logger::LogLevel::Info);
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

          virtual void
              OnUdpProtocolEvent(UdpProtocol::Event& evt, PlayerHandle handle)
          {
              Event info;

              switch (evt.type)
              {
              case UdpProtocol::Event::Connected:
                  info.code = EventCode::ConnectedToPeer;
                  info.u.connected.player = handle;
                  _callbacks.on_event(&info);
                  break;

              case UdpProtocol::Event::Synchronizing:
                  info.code = EventCode::SynchronizingWithPeer;
                  info.u.synchronizing.player = handle;
                  info.u.synchronizing.count = evt.u.synchronizing.count;
                  info.u.synchronizing.total = evt.u.synchronizing.total;
                  _callbacks.on_event(&info);
                  break;

              case UdpProtocol::Event::Synchronzied:
                  info.code = EventCode::SynchronizedWithPeer;
                  info.u.synchronized.player = handle;
                  _callbacks.on_event(&info);

                  CheckInitialSync();
                  break;

              case UdpProtocol::Event::NetworkInterrupted:
                  info.code = EventCode::ConnectionInterrupted;
                  info.u.connection_interrupted.player = handle;
                  info.u.connection_interrupted.disconnect_timeout = evt.u.network_interrupted.disconnect_timeout;
                  _callbacks.on_event(&info);
                  break;

              case UdpProtocol::Event::NetworkResumed:
                  info.code = EventCode::ConnectionResumed;
                  info.u.connection_resumed.player = handle;
                  _callbacks.on_event(&info);
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
                      logger->LogAndPrint(format("setting remote connect status for queue {} to {}", queue, evt.u.input.input.frame), "p2p.cpp", Logger::LogLevel::Info);
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
 













 /* -----------------------------------------------------------------------
 * .net (http://ggpo.net)  -  Copyright 2009 GroundStorm Studios, LLC.
 *
 * Use of this software is governed by the MIT license that can be found
 * in the LICENSE file.
 */

namespace GGPO
{
     ErrorCode
         ggpo_start_session
         (
             Session** session,
             const char* game,
             int num_players,
             int input_size,
             unsigned short localport
         )
     {
         *session = (Session*)new Peer2PeerBackend
         (
             game,
             localport,
             num_players,
             input_size
         );

         return ErrorCode::OK;
     }

     ErrorCode
         ggpo_add_player
         (
             Session* ggpo,
             Player* player,
             PlayerHandle* handle
         )
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }

         return ggpo->AddPlayer(player, handle);
     }



     ErrorCode
         ggpo_start_synctest
         (
             Session** ggpo,
             char* game,
             int num_players,
             int input_size,
             int frames
         )
     {
         *ggpo = (Session*)new SyncTestBackend(game, frames, num_players);
         return ErrorCode::OK;
     }

     ErrorCode
         ggpo_set_frame_delay
         (
             Session* ggpo,
             PlayerHandle player,
             int frame_delay
         )
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }

         return ggpo->SetFrameDelay(player, frame_delay);
     }

     ErrorCode
         ggpo_idle
         (
             Session* ggpo,
             int timeout
         )
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->DoPoll(timeout);
     }

     ErrorCode
         ggpo_add_local_input
         (
             Session* ggpo,
             PlayerHandle player,
             void* values,
             int size
         )
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->AddLocalInput(player, values, size);
     }

     ErrorCode
         ggpo_synchronize_input
         (
             Session* ggpo,
             void* values,
             int size,
             int* disconnect_flags
         )
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->SyncInput(values, size, disconnect_flags);
     }

     ErrorCode
         ggpo_disconnect_player
         (
             Session* ggpo,
             PlayerHandle player
         )
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->DisconnectPlayer(player);
     }

     ErrorCode
         ggpo_advance_frame(Session* ggpo)
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->IncrementFrame();
     }

     ErrorCode
         ggpo_client_chat(Session* ggpo, char* text)
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->Chat(text);
     }

     ErrorCode
         ggpo_get_network_stats
         (
             Session* ggpo,
             PlayerHandle player,
             NetworkStats* stats
         )
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->GetNetworkStats(stats, player);
     }


     ErrorCode
         ggpo_close_session(Session* ggpo)
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         delete ggpo;
         return ErrorCode::OK;
     }

     ErrorCode
         ggpo_set_disconnect_timeout(Session* ggpo, int timeout)
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->SetDisconnectTimeout(timeout);
     }

     ErrorCode
         ggpo_set_disconnect_notify_start(Session* ggpo, int timeout)
     {
         if (not ggpo)
         {
             return ErrorCode::INVALID_SESSION;
         }
         return ggpo->SetDisconnectNotifyStart(timeout);
     }

     ErrorCode
         ggpo_start_spectating
         (
             Session** session,
             const char* game,
             int num_players,
             int input_size,
             unsigned short local_port,
             char* host_ip,
             unsigned short host_port
         )
     {
         *session = (Session*) new SpectatorBackend
         (
             game,
             local_port,
             num_players,
             input_size,
             host_ip,
             host_port
         );
         return ErrorCode::OK;
     }
}

