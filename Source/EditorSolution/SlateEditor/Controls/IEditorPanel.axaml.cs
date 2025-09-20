using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Markup.Xaml;

namespace SlateEditor.Controls;


public partial class IEditorPanel : StackPanel
{
    public IEditorPanel()
    {
        InitializeComponent();
        DataContext = this;
    }
    
    public static readonly StyledProperty<string> TitleProperty =
        AvaloniaProperty.Register<IEditorPanel, string>(nameof(Title), "Unnamed Title");
    public string Title
    {
        get => GetValue(TitleProperty);
        set => SetValue(TitleProperty, value);
    }
}