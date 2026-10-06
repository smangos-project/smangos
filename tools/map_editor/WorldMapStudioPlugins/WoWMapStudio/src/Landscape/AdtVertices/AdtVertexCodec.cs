using System;
using System.Numerics;
using System.Runtime.InteropServices;

namespace WorldMapStudio.WoW;

/// <summary>Which imported vertex raster an edit works on.</summary>
public enum AdtVertexKind
{
    Height,

    /// <summary>Vertex colour (MCCV): a per-channel multiplier of the ground texture.</summary>
    Color,

    /// <summary>Vertex light (MCLV): light added on top of the ground.</summary>
    Light,
}

/// <summary>
/// Converts between the bytes of one texel of an imported vertex raster and the value an edit works with.
/// Values are three floats: a height uses only <c>X</c>; colour and light use R, G, B. The stored bytes are the
/// raw on-disk ones, so they are read with the client's meaning, not the preview material's.
/// </summary>
public abstract class AdtVertexCodec
{
    public static readonly AdtVertexCodec Height = new HeightCodec();
    public static readonly AdtVertexCodec Color = new ColorCodec();
    public static readonly AdtVertexCodec Light = new LightCodec();

    /// <summary>The stored byte that means "no change" in a colour channel.</summary>
    public const byte NeutralColorByte = 0x7F;

    public static AdtVertexCodec For(AdtVertexKind kind) => kind switch
    {
        AdtVertexKind.Height => Height,
        AdtVertexKind.Color => Color,
        _ => Light,
    };

    public abstract AdtVertexKind Kind { get; }

    /// <summary>Bytes per texel in the image.</summary>
    public abstract int Stride { get; }

    /// <summary>The value of a vertex nothing has touched: no colour change, no light. Heights have none.</summary>
    public abstract Vector3 Neutral { get; }

    /// <summary>The channel range a value is clamped to.</summary>
    public abstract Vector3 Clamp(Vector3 value);

    /// <summary>Reads a texel. False when the texel carries no data.</summary>
    public abstract bool TryDecode(ReadOnlySpan<byte> texel, out Vector3 value);

    public abstract void Encode(Vector3 value, Span<byte> texel);

    /// <summary>Fills a texel that carries no data with <see cref="Neutral"/>, so a raster a chunk has only just
    /// started to use holds a defined value everywhere.</summary>
    public virtual bool Seed(Span<byte> texel) => false;

    private sealed class HeightCodec : AdtVertexCodec
    {
        public override AdtVertexKind Kind => AdtVertexKind.Height;

        public override int Stride => sizeof(float);

        public override Vector3 Neutral => Vector3.Zero;

        public override Vector3 Clamp(Vector3 value) => new(value.X, 0.0f, 0.0f);

        public override bool TryDecode(ReadOnlySpan<byte> texel, out Vector3 value)
        {
            value = new Vector3(MemoryMarshal.Read<float>(texel), 0.0f, 0.0f);
            return true;
        }

        public override void Encode(Vector3 value, Span<byte> texel) => MemoryMarshal.Write(texel, in value.X);
    }

    // Multiplier m = byte / 127, so the on-disk neutral 0x7F is exactly 1 and 0xFF is a little over 2.
    // Alpha is the paint weight the preview function blends by: 255 where a texel holds colour, 0 where it
    // holds nothing.
    private sealed class ColorCodec : AdtVertexCodec
    {
        private const float Unit = 127.0f;
        private const float Max = 255.0f / Unit;

        public override AdtVertexKind Kind => AdtVertexKind.Color;

        public override int Stride => 4;

        public override Vector3 Neutral => Vector3.One;

        public override Vector3 Clamp(Vector3 value) => Vector3.Clamp(value, Vector3.Zero, new Vector3(Max));

        public override bool TryDecode(ReadOnlySpan<byte> texel, out Vector3 value)
        {
            value = new Vector3(texel[0], texel[1], texel[2]) / Unit;
            return texel[3] != 0;
        }

        public override void Encode(Vector3 value, Span<byte> texel)
        {
            texel[0] = ToByte(value.X);
            texel[1] = ToByte(value.Y);
            texel[2] = ToByte(value.Z);
            texel[3] = 255;
        }

        public override bool Seed(Span<byte> texel)
        {
            if (texel[3] != 0)
            {
                return false;
            }

            Encode(Neutral, texel);
            return true;
        }

        private static byte ToByte(float channel) => (byte)Math.Clamp((int)MathF.Round(channel * Unit), 0, 255);
    }

    // Light l = byte / 255, added to the ground. A texel of zero is no light, which is also an untouched one.
    private sealed class LightCodec : AdtVertexCodec
    {
        public override AdtVertexKind Kind => AdtVertexKind.Light;

        public override int Stride => 4;

        public override Vector3 Neutral => Vector3.Zero;

        public override Vector3 Clamp(Vector3 value) => Vector3.Clamp(value, Vector3.Zero, Vector3.One);

        public override bool TryDecode(ReadOnlySpan<byte> texel, out Vector3 value)
        {
            value = new Vector3(texel[0], texel[1], texel[2]) / 255.0f;
            return true;
        }

        public override void Encode(Vector3 value, Span<byte> texel)
        {
            texel[0] = ToByte(value.X);
            texel[1] = ToByte(value.Y);
            texel[2] = ToByte(value.Z);
            texel[3] = 0;
        }

        private static byte ToByte(float channel) => (byte)Math.Clamp((int)MathF.Round(channel * 255.0f), 0, 255);
    }
}
