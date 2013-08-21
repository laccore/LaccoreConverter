//
// Magick++ demo to generate a simple text button
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2003
// 

#include "stdafx.h"

#include <Shlwapi.h>
#include <Magick++.h>
#include <string>
#include <iostream>

#include "convert.h"

using namespace Magick;

static FILE *logfile = NULL;

void log(std::string blah)
{
	if (!logfile)
	{
		logfile = fopen("_error.log", "wt");
	}
	fprintf(logfile, "%s\n", blah.c_str());
	fflush(logfile);
}

void closeLog()
{
	if (logfile)
		fclose(logfile);
}

std::string getAppPath(const bool stripExe = true);

static std::string stripExtension(std::string filename)
{
	size_t lastDotIndex = filename.find_last_of('.');
	if (lastDotIndex == std::string::npos)
		return std::string();
	else
		return filename.substr(0, lastDotIndex);
}

BOOL dirExists(std::string szPath)
{
  DWORD dwAttrib = GetFileAttributes(szPath.c_str());

  return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
         (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void createDir(std::string basePath, std::string dirToCreate)
{
	std::string fullPath = basePath + dirToCreate;
	if (!dirExists(fullPath))
	{
		if (CreateDirectory(fullPath.c_str(), NULL) != TRUE)
		{
			if (GetLastError() != ERROR_ALREADY_EXISTS)
			{
				MessageBox(NULL, "Couldn't create directory", "Shucks", MB_OK | MB_ICONERROR);
			}
		}
	}
}

static const std::string jpegDir("\\_fullSizeJpegs\\");
static const std::string tiffDir("\\_fullSizeTiffs\\");
static const std::string scaledDir("\\_reducedJpegs\\");
void createOutputDirs()
{
	std::string appPath = getAppPath();
	createDir(appPath, tiffDir);
	createDir(appPath, jpegDir);
	createDir(appPath, scaledDir);
}

// remove [app].exe by default
std::string getAppPath(const bool stripExe /*= true*/)
{
	// get application path
	char buf[MAX_PATH];
	GetModuleFileName(NULL, buf, MAX_PATH);
	std::string appPath(buf);
	if (stripExe)
	{
		const int exeSlashIdx = appPath.find_last_of("\\");
		appPath = appPath.substr(0, exeSlashIdx);
	}
	
	return appPath;
}

bool processImage(const std::string &filename, const std::string &rulername, const int dpi, const int vertScaling)
{
	Image image;
	try {
		image.read(filename);
		Geometry origImageSize = image.size();
		//printf("image size: (%d, %d)\n", imageSize.width(), imageSize.height());
		image.rotate(-90.0);

		// remove .36 inches from top (now the left since we rotated)
		const int chopWidth = 0.36f * dpi;
		Geometry chopGeo(chopWidth, 0);
		image.chop(chopGeo);

		std::string rulerRelPath = "rulers/" + rulername;
		Image rulerOverlay;
		rulerOverlay.read(rulerRelPath);

		// rotated the image, swap height and width
		const int rulerHeight = rulerOverlay.size().height();
		Geometry extentGeometry(origImageSize.height() - chopWidth, origImageSize.width() + rulerHeight);
		image.extent(extentGeometry);

		image.composite(rulerOverlay, SouthWestGravity);

		// rename output file to match parent directory name
		int lastSlashIdx = filename.find_last_of("\\");
		const std::string pathSansFile = filename.substr(0, lastSlashIdx);
		lastSlashIdx = pathSansFile.find_last_of("\\");
		const std::string parentDirName = pathSansFile.substr(lastSlashIdx + 1);
		const std::string fileNoExt = parentDirName;

		// may want an option to use original filename rather than parent dir
		//std::string fileNoPath = filename.substr(lastSlashIdx + 1);
		//const int lastDotIdx = fileNoPath.find_last_of(".");
		//std::string fileNoExt = fileNoPath.substr(0, lastDotIdx);

		std::string tiffname, jpegname, scaledname;
		tiffname = jpegname = scaledname = getAppPath();

		if (!fileNoExt.empty())
		{
			tiffname += tiffDir;
			tiffname += fileNoExt;
			tiffname += ".tif";

			jpegname += jpegDir;
			jpegname += fileNoExt;
			jpegname += ".jpg";
			
			image.write(tiffname);
			image.write(jpegname);

			// reduced, vertical-ized image for Illustrator, PSICAT
			image.rotate(90.0);

			char scaleBuf[16];
			sprintf(scaleBuf, "%d%%", vertScaling);
			image.scale(Geometry(scaleBuf));

			scaledname += scaledDir;
			scaledname += fileNoExt;
			scaledname += ".jpg";

			image.write(scaledname);
		}
	} catch (Exception &e) {
		// so gross.
		char buf[512];
		sprintf(buf, "ImageMagick exception processing %s: %s\n", filename.c_str(), e.what());
		log(std::string(buf));
		return false;
	}

	return true;
}
