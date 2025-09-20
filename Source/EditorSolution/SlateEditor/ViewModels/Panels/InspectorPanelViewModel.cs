using System;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.ComponentModel;
using Google.Protobuf;
using Engine;


namespace SlateEditor.ViewModels.Panels;

public static class ProtoSocketHelper
{
    public static async Task SendAsync(NetworkStream stream, IMessage message, CancellationToken token)
    {
        byte[] payload = message.ToByteArray();
        byte[] sizeBytes = BitConverter.GetBytes(IPAddress.HostToNetworkOrder(payload.Length));
        await stream.WriteAsync(sizeBytes, 0, 4, token);
        await stream.WriteAsync(payload, 0, payload.Length, token);
    }
    public static async Task<byte[]?> ReceiveAsync(NetworkStream stream, CancellationToken token)
    {
        byte[] sizeBytes = new byte[4];
        int read = await stream.ReadAsync(sizeBytes, 0, 4, token);
        if (read < 4)
            return null;

        if (BitConverter.IsLittleEndian)
            Array.Reverse(sizeBytes);

        int size = BitConverter.ToInt32(sizeBytes, 0);
        byte[] buffer = new byte[size];
        int offset = 0;
        while (offset < size)
        {
            int bytesRead = await stream.ReadAsync(buffer, offset, size - offset, token);
            if (bytesRead == 0)
                return null;
            offset += bytesRead;
        }
        return buffer;
    }
}

public class SocketConnection
{
    private Socket _socket;
    private NetworkStream _stream;
    private CancellationTokenSource _cts;
    
    public event Action<IMessage>? OnMessageReceived;

    public SocketConnection(Socket socket)
    {
        _socket = socket;
        _stream = new NetworkStream(socket);
        _cts = new CancellationTokenSource();
        _ = Task.Run(() => ReceiveLoop(_cts.Token));
    }
    public void Disconnect()
    {
        _cts.Cancel();
        _stream.Close();
        _socket.Close();
    }
    
    private async Task ReceiveLoop(CancellationToken token)
    {
        try
        {
            while (!token.IsCancellationRequested)
            {
                var buffer = await ProtoSocketHelper.ReceiveAsync(_stream, token);
                if (buffer == null)
                {
                    Console.WriteLine("Connection closed by client or invalid message.");
                    break;
                }

                var message = TryParse(buffer);
                if (message != null)
                {
                    OnMessageReceived?.Invoke(message);
                }
                else
                {
                    Console.WriteLine("Failed to parse protobuf message.");
                }
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Server receive loop error: {ex.Message}");
        }
    }
    public async Task SendAsync(IMessage message)
    {
        await ProtoSocketHelper.SendAsync(_stream, message, _cts.Token);
    }
    private IMessage? TryParse(byte[] buffer)
    {
        try
        {
            return Engine.LoadSceneRequest.Parser.ParseFrom(buffer);
        }
        catch (Exception e)
        {
            Console.WriteLine("TryParse failed: " + e.Message);
            return null;
        }
    }
    
}
public class ServerSocket
{
    private Socket? _socket;
    private CancellationTokenSource? _cts;

    public event Action<SocketConnection>? OnClientConnected; 

    public void Start(string path)
    {
        if (File.Exists(path))
            File.Delete(path);
        
        _socket = new Socket(AddressFamily.Unix, SocketType.Stream, ProtocolType.Unspecified);
        _socket.Bind(new UnixDomainSocketEndPoint(path));
        _socket.Listen(10);
        _cts = new CancellationTokenSource();
        
        _ = Task.Run(() => AcceptLoop(_cts.Token));
        
        Console.WriteLine($"Server started and listening on '{path}'");
    }
    public void Stop()
    {
        _cts?.Cancel();
        _socket?.Close();
    }

    private async Task AcceptLoop(CancellationToken token)
    {
        Console.WriteLine("Server accept loop started");
        while (!token.IsCancellationRequested)
        {
            try
            {
                var clientSocket = await _socket!.AcceptAsync(token);
                Console.WriteLine("Client connected!");
                var client = new SocketConnection(clientSocket);
                OnClientConnected?.Invoke(client);
            }
            catch (OperationCanceledException)
            {
                Console.WriteLine("Server accept loop canceled.");
                break;
            }
            catch (Exception ex)
            {
                Console.WriteLine("Accept error: " + ex.Message);
            }
        }
    }
}
public class ClientSocket
{
    private Socket? _socket;
    private NetworkStream? _stream;
    private CancellationTokenSource? _cts;

    public event Action<IMessage>? OnMessageReceived;

    public async Task<bool> Connect(string filepath)
    {
        while (!File.Exists(filepath))
        {
            Console.WriteLine("Waiting for socket file...");
            await Task.Delay(150);
        }

        try
        {
            _socket = new Socket(AddressFamily.Unix, SocketType.Stream, ProtocolType.Unspecified);
            var endpoint = new UnixDomainSocketEndPoint(filepath);
            await _socket.ConnectAsync(endpoint);
            Console.WriteLine("Connected to socket!");

            _stream = new NetworkStream(_socket);
            _cts = new CancellationTokenSource();

            _ = Task.Run(() => ReceiveLoop(_cts.Token));
            return true;
        }
        catch (Exception e)
        {
            Console.WriteLine("Failed to connect to: " + filepath + ", Error: " + e.Message);
            return false;
        }
    }
    public void Disconnect()
    {
        _cts?.Cancel();
        _stream?.Close();
        _socket?.Close();
    }

    private async Task SendMessageAsync(IMessage message, NetworkStream stream, CancellationToken cancellationToken)
    {
        if (_stream == null || !_socket!.Connected)
            return;

        var payload = message.ToByteArray();
        var sizeBytes = BitConverter.GetBytes(IPAddress.HostToNetworkOrder(payload.Length));

        await _stream.WriteAsync(sizeBytes, 0, 4);
        await _stream.WriteAsync(payload, 0, payload.Length);
    }

    public async Task SendAsync(IMessage message)
    {
        if (_stream == null || !_socket!.Connected)
            return;

        await SendMessageAsync(message, _stream, _cts!.Token);
    }

    private static async Task<byte[]?> ReceiveAsync(NetworkStream stream, CancellationToken token)
    {
        var sizeBytes = new byte[4];
        Console.WriteLine("Waiting to read message size...");
        var read = await stream.ReadAsync(sizeBytes, 0, 4, token);
        Console.WriteLine($"Read {read} bytes from stream");
        if (read < 4)
        {
            Console.WriteLine("Socket closed or partial size prefix.");
            return null;
        }

        if (BitConverter.IsLittleEndian)
            Array.Reverse(sizeBytes);

        var size = BitConverter.ToInt32(sizeBytes, 0);
        var buffer = new byte[size];

        var offset = 0;
        while (offset < size)
        {
            var bytesRead = await stream.ReadAsync(buffer, offset, size - offset, token);
            if (bytesRead == 0)
            {
                Console.WriteLine("Socket closed during message read.");
                return null;
            }

            offset += bytesRead;
        }

        return buffer;
    }

    private async Task ReceiveLoop(CancellationToken token)
    {
        try
        {
            var stream = _stream!;
            while (!token.IsCancellationRequested)
            {
                var buffer = await ReceiveAsync(stream, token);
                if (buffer == null)
                {
                    Console.WriteLine("Connection closed by server.");
                    break;
                }
                var msg = TryParse(buffer);
                if (msg != null) 
                    OnMessageReceived?.Invoke(msg);
            }
        }
        catch (Exception e)
        {
            Console.WriteLine($"Socket receive loop terminated: {e.Message}");
        }
    }

    private IMessage? TryParse(byte[] data)
    {
        try
        {
            // Example: Assume wrapper message is used
            return LoadSceneRequest.Parser.ParseFrom(data);
        }
        catch
        {
            return null;
        }
    }

}

public partial class InspectorPanelViewModel : BaseViewModel
{
    private ServerSocket? _serverSocket;
    public void InitializeAsync()
    {
        _serverSocket = new ServerSocket();
        _serverSocket.OnClientConnected += client =>
        {
            client.OnMessageReceived += msg =>
            {
                Console.WriteLine("Received message from client: " + msg);
                BindContent = msg.ToString();
            };
        };
        _serverSocket.Start("/tmp/ipc.sock");
    }


    [ObservableProperty] private string _bindContent = "Waiting...";
}