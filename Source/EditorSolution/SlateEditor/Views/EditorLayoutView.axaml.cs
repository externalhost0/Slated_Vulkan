using System.Collections.ObjectModel;
using System.Linq;
using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using SlateEditor.ViewModels;

namespace SlateEditor.Views;

public partial class EditorLayoutView : UserControl
{
    public EditorLayoutView()
    {
        InitializeComponent();
        DataContext = new EditorLayoutViewModel();
    }
}