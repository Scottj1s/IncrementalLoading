#pragma once

#include "MainWindow.g.h"

namespace winrt::IncrementalLoadingCpp::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();
    };
}

namespace winrt::IncrementalLoadingCpp::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
