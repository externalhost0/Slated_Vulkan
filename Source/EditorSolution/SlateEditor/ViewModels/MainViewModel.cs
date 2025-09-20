
using System.Windows.Input;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;

namespace SlateEditor.ViewModels;

public partial class MainViewModel : BaseViewModel
{
    public string Greeting => "Welcome to Slate Editor!";
    
    public void CutCommand() { }

    public void CopyCommand() { }

    public void PasteCommand() { }
}