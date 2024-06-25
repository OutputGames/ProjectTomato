

using System.Text;
using System.IO;
using System;

using System.Runtime.InteropServices;

public class TomatoApp
{
    [DllImport("TomatoRuntime.dll", EntryPoint = "tmeInit", CallingConvention=CallingConvention.Cdecl, CharSet = CharSet.Auto)] public static extern void TomatoInit(int width, int height, string name);
    [DllImport("TomatoRuntime.dll", EntryPoint = "tmeStartLoop", CallingConvention=CallingConvention.Cdecl)] public static extern void TomatoStart();

    [DllImport("TomatoRuntime.dll", EntryPoint = "tmeLoad", CallingConvention=CallingConvention.Cdecl)] public static extern void TomatoLoad(string data);

    public static void Main(string[] args)

    {

        var gamePath = "./game.tmg";

        var gameData = File.ReadAllText(gamePath);

        Console.WriteLine("", gameData);

        TomatoInit(800,600, "TomatoApp");

        TomatoLoad(gameData);

        TomatoStart();

    }

}