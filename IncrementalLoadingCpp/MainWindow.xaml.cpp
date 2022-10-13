#include "pch.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <wil/cppwinrt_helpers.h>

using namespace winrt;
using namespace IncrementalLoadingCpp::implementation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Data;
using namespace Microsoft::UI::Xaml::Interop;
using namespace Microsoft::UI::Dispatching;
using namespace std::chrono;

namespace IncrementalCollections
{
    using namespace winrt::impl;
    namespace wfc = Windows::Foundation::Collections;

    // ISupportIncrementalLoading implemented by subclass ...
    template <typename D, typename Container = std::vector<Windows::Foundation::IInspectable>, typename ThreadingBase = single_threaded_collection_base>
    struct incremental_inspectable_observable_vector :
        observable_vector_base<inspectable_observable_vector<Container, ThreadingBase>, Windows::Foundation::IInspectable>,
        implements<D, ISupportIncrementalLoading,
        wfc::IObservableVector<Windows::Foundation::IInspectable>, wfc::IVector<Windows::Foundation::IInspectable>, wfc::IVectorView<Windows::Foundation::IInspectable>, wfc::IIterable<Windows::Foundation::IInspectable>>,
        ThreadingBase
    {
        static_assert(std::is_same_v<Container, std::remove_reference_t<Container>>, "Must be constructed with rvalue.");

        explicit incremental_inspectable_observable_vector(Container&& values) : m_values(std::forward<Container>(values))
        {
        }

        explicit incremental_inspectable_observable_vector()
        {
        }

        auto& get_container() noexcept
        {
            return m_values;
        }

        auto& get_container() const noexcept
        {
            return m_values;
        }

        using ThreadingBase::acquire_shared;
        using ThreadingBase::acquire_exclusive;


    private:

        Container m_values;
    };
}

namespace winrt::IncrementalLoadingCpp::implementation
{
    struct MyCollection : IncrementalCollections::incremental_inspectable_observable_vector<MyCollection>
    {
        static uint32_t const _itemCount{ 50 };
        static uint32_t const _pageCount{ 5 };
        DispatcherQueue _dispatcherQueue;

        MyCollection(DispatcherQueue dispatcherQueue) : _dispatcherQueue(dispatcherQueue) {}

        uint32_t Count() const
        {
            return get_container().size();
        }

        bool HasMoreItems() const
        {
            return Count() < _itemCount;
        }

        void LoadMoreItems(uint32_t const count)
        {
            for (size_t i = 0; i < count; ++i)
            {
                Append(winrt::box_value(Count()));
            }
        }

        // It's not strictly necessary to update the observable collection on a background thread.
        // But it is necessary to sequence continuation back onto the foreground thread, 
        // so that it's processed after any updates to the logical tree, behaving similarly
        // to the C# sample.  And to do that, we need a context switch away from LoadMoreItems.
        IAsyncOperation<LoadMoreItemsResult> LoadMoreItemsAsync(uint32_t const count)
        {
            co_await resume_background();
            LoadMoreItems(_pageCount /*count*/);
            co_await wil::resume_foreground(_dispatcherQueue);
            co_return LoadMoreItemsResult{ _pageCount /*count*/ };
        }
    };

    MainWindow::MainWindow()
    {
        InitializeComponent();
        MyListView().ItemsSource(winrt::make<MyCollection>(DispatcherQueue()));
    }
}
