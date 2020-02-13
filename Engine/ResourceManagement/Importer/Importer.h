#ifndef _IMPORTER_H_
#define _IMPORTER_H_
#include "Main/Globals.h"
#include "Module/ModuleFileSystem.h"

#include <algorithm>
class File;
class Importer {
public:
	Importer() = default;
	virtual ~Importer() = default;
	virtual std::pair<bool, std::string> Import(const File & file) const;

protected:

	std::string GetAlreadyImportedResource(const File & file_to_look_for) const;
	void SaveMetaFile(const File & imported_file, const std::string & exported_path) const;

	std::string GetUIDFromMeta(const File& file) const;
	std::string GetMetaFilePath(const File& file) const;

};
#endif // !_IMPORTER_H_

