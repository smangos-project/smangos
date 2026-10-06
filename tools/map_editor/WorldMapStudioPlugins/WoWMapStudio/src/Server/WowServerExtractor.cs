using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace WorldMapStudio.WoW;

/// <summary>What to hand the extractor: one map, optionally limited to one tile (<c>MMMRRCC</c> row and column).</summary>
public sealed record WowServerExtractRequest(int MapId, int? Row = null, int? Col = null, string Stages = "map,vmap,mmap", bool Force = false);

/// <summary>The finished extractor process: exit code and every output line.</summary>
public sealed record WowServerExtractResult(int ExitCode, IReadOnlyList<string> Lines);

/// <summary>Runs <c>map_extractor</c> to rebuild server data, and reads its <c>--dry-run</c> report as staleness.</summary>
public static partial class WowServerExtractor
{
    public static List<string> BuildArguments(WowServerExtractRequest request, string input, string output, bool dryRun)
    {
        var args = new List<string>
        {
            "--input", input,
            "--output", output,
            "--stages", request.Stages,
            "--map", request.MapId.ToString(CultureInfo.InvariantCulture),
        };

        if (request.Row is { } row && request.Col is { } col)
        {
            args.Add("--tile");
            args.Add(FormattableString.Invariant($"{row},{col}"));
        }

        if (request.Force)
        {
            args.Add("--force");
        }

        if (dryRun)
        {
            args.Add("--dry-run");
        }

        return args;
    }

    /// <summary>The <c>(row, col)</c> tiles a dry run says would be redone, from lines like <c>map 0 tile 48,32: would redo (…)</c>.</summary>
    public static IReadOnlyList<(int Row, int Col)> ParseStaleTiles(IEnumerable<string> lines)
    {
        var tiles = new List<(int, int)>();
        foreach (string line in lines)
        {
            Match match = StaleTile().Match(line);
            if (match.Success)
            {
                tiles.Add((int.Parse(match.Groups[1].Value, CultureInfo.InvariantCulture), int.Parse(match.Groups[2].Value, CultureInfo.InvariantCulture)));
            }
        }

        return tiles;
    }

    public static Task<WowServerExtractResult> RunAsync(string exe, IReadOnlyList<string> args) => Task.Run(() =>
    {
        var info = new ProcessStartInfo(exe)
        {
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            RedirectStandardInput = true,
            UseShellExecute = false,
        };
        foreach (string arg in args)
        {
            info.ArgumentList.Add(arg);
        }

        var lines = new List<string>();
        using Process process = Process.Start(info) ?? throw new InvalidOperationException($"Could not start {exe}");
        process.StandardInput.Close();
        void Collect(object _, DataReceivedEventArgs e)
        {
            if (e.Data != null)
            {
                lock (lines)
                {
                    lines.Add(e.Data);
                }
            }
        }

        process.OutputDataReceived += Collect;
        process.ErrorDataReceived += Collect;
        process.BeginOutputReadLine();
        process.BeginErrorReadLine();
        process.WaitForExit();
        lock (lines)
        {
            return new WowServerExtractResult(process.ExitCode, [.. lines]);
        }
    });

    [GeneratedRegex(@"tile (\d+),(\d+): would redo")]
    private static partial Regex StaleTile();
}
