// Copyright Natali Caggiano. All Rights Reserved.

#include "LogCapture.h"
#include "Misc/OutputDeviceRedirector.h"

FLogCapture& FLogCapture::Get()
{
	static FLogCapture Instance;
	return Instance;
}

void FLogCapture::StartCapturing()
{
	if (bCapturing)
	{
		return;
	}

	if (GLog)
	{
		GLog->AddOutputDevice(this);
		bCapturing = true;
	}
}

void FLogCapture::StopCapturing()
{
	if (!bCapturing)
	{
		return;
	}

	if (GLog)
	{
		GLog->RemoveOutputDevice(this);
	}

	bCapturing = false;
}

void FLogCapture::Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category)
{
	// Format exactly like the UE Output Log panel displays it
	// e.g. "LogPython: Error: name 'unreal' is not defined"
	const TCHAR* VerbosityStr = TEXT("");
	switch (Verbosity)
	{
		case ELogVerbosity::Fatal:   VerbosityStr = TEXT("Fatal");   break;
		case ELogVerbosity::Error:   VerbosityStr = TEXT("Error");   break;
		case ELogVerbosity::Warning: VerbosityStr = TEXT("Warning"); break;
		case ELogVerbosity::Display: VerbosityStr = TEXT("Display"); break;
		case ELogVerbosity::Log:     VerbosityStr = TEXT("Log");     break;
		case ELogVerbosity::Verbose: VerbosityStr = TEXT("Verbose"); break;
		default:                     VerbosityStr = TEXT("VeryVerbose"); break;
	}

	FString Line;
	if (Verbosity == ELogVerbosity::Log)
	{
		// Most common case — matches UE Output Log display (no verbosity prefix for Log)
		Line = FString::Printf(TEXT("%s: %s"), *Category.ToString(), Data);
	}
	else
	{
		Line = FString::Printf(TEXT("%s: %s: %s"), *Category.ToString(), VerbosityStr, Data);
	}

	FScopeLock Lock(&BufferLock);

	LineBuffer.Add(MoveTemp(Line));
	TotalLines++;

	// Evict oldest lines when buffer is full
	if (LineBuffer.Num() > MaxBufferLines)
	{
		LineBuffer.RemoveAt(0, LineBuffer.Num() - MaxBufferLines, EAllowShrinking::No);
	}
}

TArray<FString> FLogCapture::GetRecentLines(int32 Count, const FString& Filter) const
{
	FScopeLock Lock(&BufferLock);

	const TArray<FString>* Source = &LineBuffer;
	TArray<FString> Filtered;

	if (!Filter.IsEmpty())
	{
		for (const FString& Line : LineBuffer)
		{
			if (Line.Contains(Filter, ESearchCase::IgnoreCase))
			{
				Filtered.Add(Line);
			}
		}
		Source = &Filtered;
	}

	const int32 StartIdx = FMath::Max(0, Source->Num() - Count);
	return TArray<FString>(Source->GetData() + StartIdx, Source->Num() - StartIdx);
}

int32 FLogCapture::GetTotalLineCount() const
{
	FScopeLock Lock(&BufferLock);
	return TotalLines;
}
