using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Data;
using System;
using System.Collections.ObjectModel;
using System.Threading;
using System.Threading.Tasks;
using Windows.Foundation;

namespace IncrementalLoadingCs
{
    // Observable collection that can load more items incrementally.
    public class MyCollection : ObservableCollection<Object>, ISupportIncrementalLoading
    {
        private uint _itemCount = 50;
        private uint _pageCount = 5;

        public bool HasMoreItems 
        {
            get => this.Count < _itemCount; 
        }

        public MyCollection()
        {
        }

        // With the 'await', this method is made async and continuations are posted to the message pump,
        // enabling the Xaml measure and arrange phases to correctly handle lazy incremental loads.
        // Without the 'await', this method runs synchronously (compiling with warning CS1998),
        // and all items are fetched eagerly instead.
        private async Task<LoadMoreItemsResult> LoadMoreItemsAsync(uint count, CancellationToken cancellationToken)
        {
            for (uint i = 0; i < count; i++)
            {
                this.Add(this.Count);
            }
            // The actual delay is irrelevant - the 'await' is needed to ensure async execution.
            await Task.Delay(1);
            return new LoadMoreItemsResult { Count = count };
        }

        public IAsyncOperation<LoadMoreItemsResult> LoadMoreItemsAsync(uint count)
            => LoadMoreItemsAsync(_pageCount /*count*/, new CancellationToken(false)).AsAsyncOperation();
    }

    /// <summary>
    /// An empty window that can be used on its own or navigated to within a Frame.
    /// </summary>
    public sealed partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            MyListView.ItemsSource = new MyCollection();
        }
    }
}
