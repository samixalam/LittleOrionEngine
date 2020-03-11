#include "ResourceDataBase.h"
#include "ResourceManagement/Importer/Importer.h"

void ResourceDataBase::AddEntry(const File& meta_file)
{
	std::unique_ptr<ImportOptions> options = std::make_unique<ImportOptions>();
	Importer::GetOptionsFromMeta(meta_file,*options);
	if (database.find(options->uuid) == database.end())
	{
		database[options->uuid] = std::move(options);
	}


}