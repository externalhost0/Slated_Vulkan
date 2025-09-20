using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using SlateEditor.ViewModels.Panels;

namespace SlateEditor.Views.Panels;

public partial class ScenePanelView : UserControl
{
    public ScenePanelView()
    {
        InitializeComponent();
        DataContext = new ScenePanelViewModel();
    }
}