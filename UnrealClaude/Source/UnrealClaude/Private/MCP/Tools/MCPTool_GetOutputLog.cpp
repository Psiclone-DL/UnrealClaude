// Copyright Natali Caggiano. All Rights Reserved.

#include "MCPTool_GetOutputLog.h"
#include "LogCapture.h"
#include "UnrealClaudeModule.h"
#include "UnrealClaudeConstants.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"

FMCPToolResult FMCPTool_GetOutputLog::Execute(const TSharedRef<FJsonObject>& Params)
{
	int32 NumLines = UnrealClaudeConstants::MCPServer::DefaultOutputLogLines;
	if (Params->HasField(TEXT("lines")))
	{
		NumLines = FMath::Clamp(static_cast<int32>(Params->GetNumberField(TEXT("lines"))), 1, UnrealClaudeConstants::MCPServer::MaxOutputLogLines);
	}

	FString Filter;
	Params->TryGetStringField(TEXT("filter"), Filter);

	// --- PRIMARY PATH: in-memory live capture (always up-to-date, no disk flush needed) ---
	FLogCapture& Capture = FLogCapture::Get();
	if (Capture.IsCapturing() && Capture.GetTotalLineCount() > 0)
	{
		TArray<FString> Lines = Capture.GetRecentLines(NumLines, Filter);
		FString LogOutput = FString::Join(Lines, TEXT("\n"));

		TSharedPtr<FJsonObject> ResultData = MakeShared<FJsonObject>();
		ResultData->SetStringField(TEXT("source"), TEXT("live_memory"));
		ResultData->SetNumberField(TEXT("total_captured"), Capture.GetTotalLineCount());
		ResultData->SetNumberField(TEXT("returned_lines"), Lines.Num());
		if (!Filter.IsEmpty())
		{
			ResultData->SetStringField(TEXT("filter"), Filter);
		}
		ResultData->SetStringField(TEXT("content"), LogOutput);

		return FMCPToolResult::Success(
			FString::Printf(TEXT("Retrieved %d live log lines (source: in-memory)"), Lines.Num()),
			ResultData
		);
	}

	// --- FALLBACK: disk log file (used if capture hasn't started yet) ---
	UE_LOG(LogUnrealClaude, Warning, TEXT("LogCapture not active — falling back to disk log"));

	FString ProjectLogDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectLogDir());
	FString EngineLogDir  = FPaths::ConvertRelativePathToFull(FPaths::EngineDir() / TEXT("Saved/Logs"));

	FString LogFilePath;
	bool bFound = false;
	TArray<FString> SearchedPaths;

	auto TryPath = [&](const FString& Candidate) -> bool
	{
		SearchedPaths.Add(Candidate);
		if (FPaths::FileExists(Candidate))
		{
			LogFilePath = Candidate;
			return true;
		}
		return false;
	};

	bFound = TryPath(ProjectLogDir / FApp::GetProjectName() + TEXT(".log"))
		  || TryPath(ProjectLogDir / TEXT("UnrealEditor.log"));

	if (!bFound)
	{
		TArray<FString> LogFiles;
		IFileManager::Get().FindFiles(LogFiles, *ProjectLogDir, TEXT("*.log"));
		if (LogFiles.Num() > 0) { LogFilePath = ProjectLogDir / LogFiles[0]; bFound = true; }
	}

	if (!bFound)
	{
		bFound = TryPath(EngineLogDir / TEXT("UnrealEditor.log"));
	}

	if (!bFound)
	{
		TArray<FString> LogFiles;
		IFileManager::Get().FindFiles(LogFiles, *EngineLogDir, TEXT("*.log"));
		if (LogFiles.Num() > 0) { LogFilePath = EngineLogDir / LogFiles[0]; bFound = true; }
	}

	if (!bFound)
	{
		return FMCPToolResult::Error(
			FString::Printf(TEXT("LogCapture not active and no log file found. Searched: %s"),
				*FString::Join(SearchedPaths, TEXT(", "))));
	}

	FString LogContent;
	if (!FFileHelper::LoadFileToString(LogContent, *LogFilePath, FFileHelper::EHashOptions::None, FILEREAD_AllowWrite))
	{
		return FMCPToolResult::Error(FString::Printf(TEXT("Failed to read log file: %s"), *LogFilePath));
	}

	TArray<FString> AllLines;
	LogContent.ParseIntoArrayLines(AllLines);

	TArray<FString> FilteredLines;
	if (Filter.IsEmpty())
	{
		FilteredLines = AllLines;
	}
	else
	{
		for (const FString& Line : AllLines)
		{
			if (Line.Contains(Filter, ESearchCase::IgnoreCase))
			{
				FilteredLines.Add(Line);
			}
		}
	}

	const int32 StartIndex = FMath::Max(0, FilteredLines.Num() - NumLines);
	TArray<FString> ResultLines(FilteredLines.GetData() + StartIndex, FilteredLines.Num() - StartIndex);
	FString LogOutput = FString::Join(ResultLines, TEXT("\n"));

	TSharedPtr<FJsonObject> ResultData = MakeShared<FJsonObject>();
	ResultData->SetStringField(TEXT("source"), TEXT("disk_fallback"));
	ResultData->SetStringField(TEXT("log_file"), LogFilePath);
	ResultData->SetNumberField(TEXT("total_lines"), AllLines.Num());
	ResultData->SetNumberField(TEXT("returned_lines"), ResultLines.Num());
	if (!Filter.IsEmpty())
	{
		ResultData->SetStringField(TEXT("filter"), Filter);
		ResultData->SetNumberField(TEXT("filtered_lines"), FilteredLines.Num());
	}
	ResultData->SetStringField(TEXT("content"), LogOutput);

	return FMCPToolResult::Success(
		FString::Printf(TEXT("Retrieved %d log lines from disk (live capture not active)"), ResultLines.Num()),
		ResultData
	);
}
