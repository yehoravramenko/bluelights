#include "pch.hpp"

#include <ws2tcpip.h>
#include <signal.h>

#include <print>
#include <iostream>
#include <cstdlib>

typedef unsigned long long u64;

constexpr byte SET_COLOR_HEADER[] = {0x7e, 0x07, 0x05, 0x03};
constexpr byte SET_COLOR_FOOTER[] = {0x10, 0xef};

u64 currentColor = 0xCCFF00;

bool exitRequest = false;

// https://github.com/DerekGn/WinRtBle/blob/master/src/WinRTBle/Main.cpp#L12
std::wstring uuidToString(GUID uuid)
{
    std::wstring uuid_out{};
    WCHAR *uuid_wchar = nullptr;

    if (::UuidToStringW(&uuid, reinterpret_cast<RPC_WSTR *>(&uuid_wchar)) !=
        RPC_S_OK)
    {
        std::println("errrr");
        std::exit(1);
    }

    uuid_out = uuid_wchar;
    ::RpcStringFreeW(reinterpret_cast<RPC_WSTR *>(&uuid_wchar));

    return uuid_out;
}

winrt::Windows::Foundation::IAsyncAction openDevice(const u64 deviceAddress)
{
    using namespace winrt::Windows::Devices::Bluetooth;

    BluetoothLEDevice device =
        co_await BluetoothLEDevice::FromBluetoothAddressAsync(deviceAddress);

    std::wcout << std::hex << "\tDevice Information: " << std::endl
               << "\tBluetoothAddress: [" << device.BluetoothAddress() << "]"
               << std::endl
               << "\tConnectionStatus: ["
               << (device.ConnectionStatus() ==
                           BluetoothConnectionStatus::Connected
                       ? "Connected"
                       : "Disconnected")
               << "]" << std::endl
               << "\tDeviceId: [" << device.DeviceId().c_str() << "]"
               << std::endl
               << std::endl;

    GenericAttributeProfile::GattDeviceServicesResult services =
        co_await device.GetGattServicesAsync();

    std::println("Services count: {}", services.Services().Size());

    // TODO: replace magic index 1 (second service is needed)
    GenericAttributeProfile::GattDeviceService dataService =
        services.Services().GetAt(1);

    std::wcout << std::hex
               << "Service - UUID: " << uuidToString(dataService.Uuid())
               << '\n';

    GenericAttributeProfile::GattCharacteristicsResult characteristics =
        co_await dataService.GetCharacteristicsAsync();

    // TODO: replace magic index 1 (second characteristic is needed)
    GenericAttributeProfile::GattCharacteristic dataCharacteristic =
        characteristics.Characteristics().GetAt(1);

    std::wcout << std::hex << "\t\t\tCharacteristic - UUID: "
               << uuidToString(dataCharacteristic.Uuid()) << '\n';

    using winrt::Windows::Storage::Streams::DataWriter;

    DataWriter writer = DataWriter();

    /*
        0xCC,
        0xFF,
        0x00,
    */

    // byte rawColor[] = {
    //     0xCC,
    //     0xFF,
    //     0x00,
    // };
    // winrt::array_view<byte const> color{rawColor};

    writer.WriteBytes(winrt::array_view<byte const>{SET_COLOR_HEADER});
    writer.WriteBytes(winrt::array_view<byte const>{(currentColor >> 16) & 0xFF,
                                                    (currentColor >> 8) & 0xFF,
                                                    currentColor & 0xFF});
    // writer.WriteUInt64(currentColor);
    writer.WriteBytes(winrt::array_view<byte const>{SET_COLOR_FOOTER});

    auto ib = writer.DetachBuffer();
    std::cout << ib.data()[0] << std::endl;

    GenericAttributeProfile::GattWriteResult res =
        co_await dataCharacteristic.WriteValueWithResultAsync(
            ib, GenericAttributeProfile::GattWriteOption::WriteWithoutResponse);

    if (res.Status() !=
        GenericAttributeProfile::GattCommunicationStatus::Success)
    {
        std::println("Error in communication");
        std::exit(1);
    }
}

extern "C" int __cdecl interruptCalled(int, int)
{
    exitRequest = true;
    return 0;
}

int main(int, char **)
{
    std::atomic<u64> btAddr = 0;

    signal(SIGINT, reinterpret_cast<_crt_signal_t>(&interruptCalled));

    using namespace winrt::Windows::Devices::Bluetooth::Advertisement;

    BluetoothLEAdvertisementWatcher bleWatcher{};

    bleWatcher.ScanningMode(BluetoothLEScanningMode::Passive);
    bleWatcher.Received(
        [&](BluetoothLEAdvertisementWatcher watcher,
            BluetoothLEAdvertisementReceivedEventArgs eventArgs) {
            winrt::hstring localName = eventArgs.Advertisement().LocalName();

            if (localName == L"ELK-BLEDOB")
            {
                watcher.Stop();
                btAddr = eventArgs.BluetoothAddress();
            }
        });

    bleWatcher.Start();

    for (int i = 0; i < 10 && btAddr == 0; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (btAddr == 0)
        return 1;

    WSADATA wsaData = {};
    int wsaerr      = 0;

    wsaerr = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (wsaerr != 0)
    {
        std::println("WSAstartup error");
        return 1;
    }

    SOCKET serverSocket = INVALID_SOCKET;

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET)
    {
        std::println("Failed to create a socket");
        return 1;
    }

    sockaddr_in service = {};

    service = {
        .sin_family = AF_INET,
        .sin_port   = htons(6969),
    };
    inet_pton(AF_INET, "127.0.0.1", &(service.sin_addr));

    if (bind(serverSocket, reinterpret_cast<SOCKADDR *>(&service),
             sizeof(service)) == SOCKET_ERROR)
    {
        std::println("Failed to bind socket");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 1) == SOCKET_ERROR)
    {
        std::println("Failed to listen");
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    for (; exitRequest != true;)
    {
        std::println("Waiting for connection ...");

        SOCKET acceptSocket = INVALID_SOCKET;
        acceptSocket        = accept(serverSocket, nullptr, nullptr);

        if (acceptSocket == INVALID_SOCKET)
        {
            std::println("Failed to accept");
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        char receivedBuf[1024];
        int byteCount = recv(acceptSocket, receivedBuf, 1024, 0);
        if (byteCount < 0)
        {
            std::println("recv Error");
            return 1;
        }

        std::string received{&(receivedBuf[byteCount - 6])};

        u64 newColor = std::stoll(received, nullptr, 16);
        if (newColor == currentColor)
            continue;

        currentColor = newColor;

        openDevice(btAddr).get();
    }

    WSACleanup();

    return 0;
}
