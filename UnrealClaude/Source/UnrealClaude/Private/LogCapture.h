// Copyright Natali Caggiano. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/OutputDevice.h"

/**
 * In-memory log capture device.
 *
 * Registers itself with GLog on StartCapturing() and receives every log line
 * the moment UE5 produces it — no disk flush required.
 *
 * Used by MCPTool_GetOutputLog to return truly live Output Log content
 * instead of reading the potentially-stale log file from disk.
 */
class FLogCapture : public FOutputDevice
{
public:
	/** Get the singleton instance */
	static FLogCapture& Get();

	/**
	 * Register with GLog. Call once from module StartupModule().
	 * Safe to call multiple times — idempotent.
	 */
	void StartCapturing();

	/**
	 * Unregister from GLog. Call from module ShutdownModule().
	 */
	void StopCapturing();

	/** Whether capture is currently active */
	bool IsCapturing() const { return bCapturing; }

	/**
	 * Return the last N captured lines, optionally filtered by substring.
	 * @param Count   Max number of lines to return (clamped to buffer size)
	 * @param Filter  Case-insensitive substring filter — empty = no filter
	 */
	TArray<FString> GetRecentLines(int32 Count, const FString& Filter = TEXT("")) const;

	/** Total number of lines captured since StartCapturing() */
	int32 GetTotalLineCount() const;

	// FOutputDevice interface — called by GLog for every log message
	virtual void Serialize(const TCHAR* Data, ELogVerbosity::Type Verbosity, const FName& Category) override;

	// Must not be serialized to disk
	virtual bool CanBeUsedOnAnyThread() const override { return true; }

private:
	FLogCapture() = default;

	/** Max lines stored in the ring buffer before oldest are evicted */
	static constexpr int32 MaxBufferLines = 8000;

	mutable FCriticalSection BufferLock;

	/** Circular log buffer — newest lines at the end */
	TArray<FString> LineBuffer;

	/** Running total of all lines ever received */
	int32 TotalLines = 0;

	bool bCapturing = false;
};
