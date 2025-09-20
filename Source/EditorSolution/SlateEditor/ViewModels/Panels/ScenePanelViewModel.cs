using System.Collections.ObjectModel;
using Avalonia.Controls;
using Avalonia.Controls.Models.TreeDataGrid;

namespace SlateEditor.ViewModels.Panels;

public class SceneObject
{
    public string? Name { get; set; } = "Untitled";
    public int ID { get; set; }
    public ObservableCollection<SceneObject> Children { get; } = new();
}

public partial class ScenePanelViewModel : BaseViewModel
{
    private ObservableCollection<SceneObject> _objects = new()
    {
        new SceneObject
        {
            Name = "Eleanor",
            ID = 32,
            Children =
            {
                new SceneObject { Name = "Marcel", ID = 4 },
            }
        },
        new SceneObject
        {
            Name = "Jeremy",
            ID = 74,
            Children =
            {
                new SceneObject
                {
                    Name = "Jane",
                    ID = 42,
                    Children =
                    {
                        new SceneObject { Name = "Lailah ", ID = 16 }
                    }
                },
            }
        },
        new SceneObject { Name = "Jazmine", ID = 52 },
    };

    public ScenePanelViewModel()
    {
        Bind_SceneObjects = new HierarchicalTreeDataGridSource<SceneObject>(_objects)
        {
            Columns =
            {
                new HierarchicalExpanderColumn<SceneObject>(
                    new TextColumn<SceneObject, string>("Name", x => x.Name),
                    x => x.Children),
                new TextColumn<SceneObject, int>("Age", x => x.ID),
            }
        };
    }

    public HierarchicalTreeDataGridSource<SceneObject> Bind_SceneObjects { get; }
}