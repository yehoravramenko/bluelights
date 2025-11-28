#include "pch.hpp"
#include <print>
#include <iostream>
#include <cstdlib>

int main(int, char **)
{
    std::println("bluelights");

    std::atomic<unsigned long long> btAddr = 0;

    using namespace winrt::Windows::Devices::Bluetooth::Advertisement;

    BluetoothLEAdvertisementWatcher bleWatcher{};

    bleWatcher.ScanningMode(BluetoothLEScanningMode::Passive);
    bleWatcher.Received(
        [&](BluetoothLEAdvertisementWatcher watcher,
            BluetoothLEAdvertisementReceivedEventArgs eventArgs) {
            winrt::hstring localName = eventArgs.Advertisement().LocalName();

            std::wcout << "AdvertisementReceived:" << std::endl
                       << "\tLocalName: ["
                       << eventArgs.Advertisement().LocalName().c_str() << "]"
                       << "\tBluetoothAddress: [0x" << std::hex
                       << eventArgs.BluetoothAddress() << "]"
                       << "\tRawSignalStrengthInDBm: [" << std::dec
                       << eventArgs.RawSignalStrengthInDBm() << "]"
                       << std::endl;

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

    return 0;
}
