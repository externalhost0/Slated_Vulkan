using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using SlateEditor.ViewModels.Panels;

namespace SlateEditor.Views.Panels;

public partial class InspectorPanelView : UserControl
{
    public InspectorPanelView()
    {
        InitializeComponent();
        DataContext = new InspectorPanelViewModel();
        
        if (DataContext is InspectorPanelViewModel vm)
        {
            vm.InitializeAsync();
        }
    }
}