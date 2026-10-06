namespace WorldMapStudio.WoW;

/// <summary>Texture paths compare without regard to case or slash direction.</summary>
public static class AdtTexturePath
{
    public static string Key(string path) => path.Replace('\\', '/').ToLowerInvariant();

    public static bool Same(string a, string b)
    {
        if (a.Length != b.Length)
        {
            return false;
        }

        for (int i = 0; i < a.Length; i++)
        {
            if (Fold(a[i]) != Fold(b[i]))
            {
                return false;
            }
        }

        return true;
    }

    private static char Fold(char c) => c == '\\' ? '/' : char.ToLowerInvariant(c);
}
