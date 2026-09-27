#include "Extract7z.h"

#include "7z.h"
#include "7zAlloc.h"
#include "7zCrc.h"
#include "7zFile.h"

#include <string.h>

static void SetError(char* errorOut, size_t errorOutSize, char const* message)
{
    if (!errorOut || errorOutSize == 0)
        return;

#if defined(_MSC_VER)
    strncpy_s(errorOut, errorOutSize, message, _TRUNCATE);
#else
    strncpy(errorOut, message, errorOutSize - 1);
    errorOut[errorOutSize - 1] = '\0';
#endif
}

int Lzmasdk_Extract7z(char const* archivePath, char const* outPath, char* errorOut, size_t errorOutSize)
{
    ISzAlloc const allocImp = { SzAlloc, SzFree };
    ISzAlloc const allocTempImp = { SzAllocTemp, SzFreeTemp };
    CFileInStream archiveStream;
    CLookToRead2 lookStream;
    CSzArEx db;
    SRes res;
    BoolInt extracted = False;

    if (InFile_Open(&archiveStream.file, archivePath) != 0)
    {
        SetError(errorOut, errorOutSize, "cannot open the archive");
        return 1;
    }

    FileInStream_CreateVTable(&archiveStream);
    archiveStream.wres = 0;
    LookToRead2_CreateVTable(&lookStream, False);
    lookStream.buf = (Byte*)ISzAlloc_Alloc(&allocImp, (size_t)1 << 18);
    if (!lookStream.buf)
    {
        File_Close(&archiveStream.file);
        SetError(errorOut, errorOutSize, "out of memory");
        return 1;
    }
    lookStream.bufSize = (size_t)1 << 18;
    lookStream.realStream = &archiveStream.vt;
    LookToRead2_INIT(&lookStream)

    CrcGenerateTable();
    SzArEx_Init(&db);

    res = SzArEx_Open(&db, &lookStream.vt, &allocImp, &allocTempImp);
    if (res == SZ_OK)
    {
        UInt32 i;
        for (i = 0; i < db.NumFiles; i++)
        {
            UInt32 blockIndex = 0xFFFFFFFF;
            Byte* outBuffer = 0;
            size_t outBufferSize = 0;
            size_t offset = 0;
            size_t outSizeProcessed = 0;

            if (SzArEx_IsDir(&db, i))
                continue;

            res = SzArEx_Extract(&db, &lookStream.vt, i,
                &blockIndex, &outBuffer, &outBufferSize,
                &offset, &outSizeProcessed,
                &allocImp, &allocTempImp);
            if (res == SZ_OK)
            {
                CSzFile outFile;
                if (OutFile_Open(&outFile, outPath) != 0)
                {
                    SetError(errorOut, errorOutSize, "cannot open the output file");
                    res = SZ_ERROR_FAIL;
                }
                else
                {
                    size_t written = outSizeProcessed;
                    if (File_Write(&outFile, outBuffer + offset, &written) != 0 || written != outSizeProcessed)
                    {
                        SetError(errorOut, errorOutSize, "cannot write the output file");
                        res = SZ_ERROR_FAIL;
                    }
                    File_Close(&outFile);
                }
                extracted = True;
            }
            else
            {
                SetError(errorOut, errorOutSize, "cannot extract the archive");
            }

            ISzAlloc_Free(&allocImp, outBuffer);
            break;
        }

        if (res == SZ_OK && !extracted)
        {
            SetError(errorOut, errorOutSize, "the archive has no files");
            res = SZ_ERROR_FAIL;
        }
    }
    else
    {
        SetError(errorOut, errorOutSize, "cannot read the archive");
    }

    SzArEx_Free(&db, &allocImp);
    ISzAlloc_Free(&allocImp, lookStream.buf);
    File_Close(&archiveStream.file);

    return res == SZ_OK ? 0 : 1;
}
