#include "pch.hpp"
#include <print>
#include <iostream>
#include <cstdlib>

typedef unsigned long long u64;

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

    byte rawArrayDataOff[] = {0x7e, 0x07, 0x05, 0x03, /* header */
                              0xff, 0x00, 0x00,       /* colors */
                              0x10, 0xef /* footer */};
    winrt::array_view<byte const> arrayDataOff{rawArrayDataOff};

    writer.WriteBytes(arrayDataOff);

    GenericAttributeProfile::GattWriteResult res =
        co_await dataCharacteristic.WriteValueWithResultAsync(
            writer.DetachBuffer(),
            GenericAttributeProfile::GattWriteOption::WriteWithoutResponse);

    if (res.Status() ==
        GenericAttributeProfile::GattCommunicationStatus::Success)
    {
        std::println("PEREMAHA");
        co_return;
    }

    // Device Name
    // L"00002a00-0000-1000-8000-00805f9b34fb"
}

int main(int, char **)
{
    std::println("bluelights");

    std::atomic<u64> btAddr = 0;

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

    openDevice(btAddr).get();

    return 0;
}
