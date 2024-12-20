#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <wx/wx.h>

#include<algorithm>
#include <zlib.h>// Include zlib for compression




namespace fs = std::filesystem;
class Folder;
// Global folder manager

extern std::vector<Folder*> folderManager;

class Folder {
    int num_recursion = 0;
#ifdef _DEBUG
    bool debugMode = true;
#else
    bool debugMode = false;
#endif

public:
    std::string name;
    Folder* parent = nullptr;
    Folder* next = nullptr;
    std::vector<Folder*> children;  // Vector to hold child folders

    Folder(const std::string& folderName)
        : name(folderName), parent(nullptr) {
        // Register the new folder in the global manager
        folderManager.push_back(this);
        if (debugMode) wxLogMessage("Created folder: %s", name.c_str());
    }

    // Add a child folder
    void addChild(Folder* child) {
        child->parent = this;
        children.push_back(child);  // Add to the vector of children
    }

    // Load children from disk
    void loadChildrenFromDisk() {
        fs::path fullPath = fs::current_path() / name;
        if (fs::exists(fullPath) && fs::is_directory(fullPath)) {
            for (const auto& entry : fs::directory_iterator(fullPath)) {
                if (entry.is_directory()) {
                    // Check if folder already exists in global manager
                    auto it = std::find_if(folderManager.begin(), folderManager.end(),
                        [&](const Folder* folder) { return folder->name == entry.path().filename().string(); });
                    if (it == folderManager.end()) {  // Only add if not already present
                        Folder* newFolder = new Folder(entry.path().filename().string());
                        addChild(newFolder);  // Add the folder as a child
                        if (debugMode) wxLogMessage("Loaded folder: %s", newFolder->name.c_str());
                    }
                }
            }
        }
        else {
            if (debugMode) wxLogMessage("Directory does not exist or is not a directory: %s", fullPath.string().c_str());
        }
    }

    // Create folder on disk
    void create(const fs::path& parentPath) const {
    // Create the current folder
    fs::path folderPath = parentPath / name;
    if (debugMode) wxLogMessage("create called: %s to %s", folderPath.string().c_str(), name.c_str());
    
    if (!fs::exists(folderPath)) {
        fs::create_directory(folderPath);
        if (debugMode) wxLogMessage("Created folder: %s", folderPath.string().c_str());
    }

    // Recursively create directories for all children
    for (const auto& child : children) {
        child->create(folderPath); // Pass the current folder's path as the parentPath for the child
    }
}


    // Create a folder within another named folder
    void createWithinNamedFolder(const char* parentName, const char* newFolderName) {
        if (debugMode) wxLogMessage("createWithinNamedFolder name:%s parent:%s child:%s children length:%zu",
            name.c_str(), parentName, newFolderName, children.size());

        if (num_recursion > 10) {
            if (debugMode) wxLogMessage("Recursion limit reached in %s", name.c_str());
            return;
        }
        if (children.empty()) {
            if (debugMode) wxLogMessage("Children empty loading from disk %s", name.c_str());
            loadChildrenFromDisk(); // Ensure children are loaded
        }

        if (name == parentName) {
            if (debugMode) wxLogMessage("Entered If Created folder: %s", name.c_str());
            Folder* newFolder = new Folder(newFolderName);
            newFolder->parent = this; // Set the parent
            newFolder->next = newFolder; // Set self-loop for next
            addChild(newFolder);

            // Construct the full path including all parent folders
            fs::path folderPath = fs::current_path();
            std::vector<std::string> folderNames;

            // Traverse up the tree to collect folder names
            const Folder* current = this;
            while (current) {
                folderNames.push_back(current->name);
                current = current->parent;
            }

            // Add the new folder name to the path
            std::reverse(folderNames.begin(), folderNames.end()); // Reverse to get the correct order
            for (const auto& folderName : folderNames) {
                folderPath /= folderName;
            }
            folderPath /= newFolderName;

            if (debugMode) wxLogMessage("New folder path: %s (exists: %d)", folderPath.string().c_str(), fs::exists(folderPath));
            try {
                if (!fs::exists(folderPath)) {
                    fs::create_directory(folderPath);
                    if (debugMode) wxLogMessage("Created folder: %s", folderPath.string().c_str());
                }
            }
            catch (const std::exception& e) {
                if (debugMode) wxLogMessage("Exception in createWithinNamedFolder: %s", e.what());
            }
        }
        else {
            num_recursion++;
            for (auto& child : children) {
                child->createWithinNamedFolder(parentName, newFolderName);
            }
            num_recursion--; // Decrement recursion counter after traversal
        }
    }


    /*void createDirectoriesFromRelativePath(const std::string& relativePath) {
        fs::path fullPath = fs::current_path()/ name / relativePath;  // Get the full path

        if (debugMode) wxLogMessage("Creating directories for path: %s", fullPath.string().c_str());

        try {
            // Create the directories recursively
            if (!fs::exists(fullPath)) {
                fs::create_directories(fullPath); // This will create all intermediate directories
                if (debugMode) wxLogMessage("Created directories: %s", fullPath.string().c_str());
            }
            else {
                if (debugMode) wxLogMessage("Directories already exist: %s", fullPath.string().c_str());
            }
        }
        catch (const std::exception& e) {
            if (debugMode) wxLogMessage("Exception in createDirectoriesFromRelativePath: %s", e.what());
        }
    }*/

   void createDirectoriesFromRelativePath(const std::string& relativePath) {
    fs::path fullPath = fs::current_path() / name / relativePath;  // Get the full path

    if (debugMode) wxLogMessage("Creating directories for path: %s", fullPath.string().c_str());

    try {
        // Create the directories recursively
        if (!fs::exists(fullPath)) {
            fs::create_directories(fullPath); // This will create all intermediate directories
            if (debugMode) wxLogMessage("Created directories: %s", fullPath.string().c_str());
        }
        else {
            if (debugMode) wxLogMessage("Directories already exist: %s", fullPath.string().c_str());
        }

        // Parse the relative path to ensure each folder is added to the folder structure
        fs::path folderPath = relativePath;
        Folder* currentFolder = this;

        for (const auto& part : folderPath) {
            // Check if part already exists in children
            auto it = std::find_if(currentFolder->children.begin(), currentFolder->children.end(),
                                   [&](Folder* child) { return child->name == part.string(); });

            if (it == currentFolder->children.end()) {
                // Create new folder and add to children if it doesn't exist
                Folder* newFolder = new Folder(part.string());
                newFolder->parent = currentFolder;
                currentFolder->addChild(newFolder);
                currentFolder = newFolder; // Move to the new folder for the next iteration
            }
            else {
                // Folder already exists, move to it
                currentFolder = *it;
            }
        }
    }
    catch (const std::exception& e) {
        if (debugMode) wxLogMessage("Exception in createDirectoriesFromRelativePath: %s", e.what());
    }
}




    bool storeFile(const std::string& relativeFilePath, const std::string& content, bool useGzip = false) {
        fs::path fullPath = fs::current_path() / name/relativeFilePath;  // Get the full path

        // Create the directory structure if it doesn't exist
        fs::path directory = fullPath.parent_path();
        try {
            if (!fs::exists(directory)) {
                fs::create_directories(directory);  // Create directories recursively
            }
        }
        catch (const std::exception& e) {
            if (debugMode) wxLogMessage("Exception while creating directories: %s", e.what());
            return false;  // Return false on failure
        }

        // Append .gz extension if using Gzip
       /* if (useGzip && fullPath.extension() != ".gz") {
            fullPath += ".gz";
        }
       if (useGzip) {
        assert(fullPath.substr(fullPath.size() - 3) == ".gz" && "Gzip file must have .gz extension");
    } else {
        assert(fullPath.substr(fullPath.size() - 3) != ".gz" && "Non-Gzip file should not have .gz extension");
    }*/
   // Assert that the file extension is correct based on whether Gzip is used
    if (useGzip) {
        assert(fullPath.extension() == ".gz" && "Gzip file must have .gz extension");
    } else {
        assert(fullPath.extension() != ".gz" && "Non-Gzip file should not have .gz extension");
    }

        if (debugMode) wxLogMessage("Storing file: %s", fullPath.string().c_str());
        
        try {
            if (useGzip) {
                // Save as Gzip
                auto outFile = gzopen(fullPath.string().c_str(), "wb");
                if (!outFile) {
                    if (debugMode) wxLogMessage("Failed to open file for writing: %s", fullPath.string().c_str());
                    return false;
                }

                // Write content to the gzip file
                int bytesWritten = gzwrite(outFile, content.c_str(), content.size());
                gzclose(outFile);

                if (bytesWritten < 0) {
                    if (debugMode) wxLogMessage("Error writing to gzip file: %s", fullPath.string().c_str());
                    return false;
                }
                if (bytesWritten != content.size()) {
                if (debugMode) wxLogMessage("Error writing to gzip file: %s", fullPath.string().c_str());
                return false;
                }

                if (debugMode) wxLogMessage("Stored file as Gzip: %s", fullPath.string().c_str());
                 return true;  // Return true after successful gzip write
            }
            else {
                // Save as regular text file
                std::ofstream outFile(fullPath, std::ios::binary);
            if (outFile.is_open()) {
                outFile.write(content.c_str(), content.size());
                outFile.close();
                if (debugMode) wxLogMessage("Stored file: %s", fullPath.c_str());
                return true;
            } else {
                if (debugMode) wxLogMessage("Failed to open file for writing: %s", fullPath.c_str());
            }
                /*std::ofstream outFile(fullPath);
                if (outFile.is_open()) {
                    outFile << content;  // Write content to file
                    outFile.close();
                    if (debugMode) wxLogMessage("Stored file: %s", fullPath.string().c_str());
                    return true;  // Return true on success
                }
                else {
                    if (debugMode) wxLogMessage("Failed to open file for writing: %s", fullPath.string().c_str());
                }*/
            }
        }
        catch (const std::exception& e) {
            if (debugMode) wxLogMessage("Exception in storeFile: %s", e.what());
        }
        return false;  // Return false on failure
    }

    bool retrieveFile(const std::string& relativeFilePath, std::string& content) {
        fs::path fullPath = fs::current_path() / name/relativeFilePath;  // Get the full path

        // Check if the file is gzipped
        bool useGzip = false;
        if (fullPath.extension() == ".gz") {
            useGzip = true;
        }

        if (debugMode) wxLogMessage("Retrieving file: %s", fullPath.string().c_str());

        content.clear();  // Clear previous content
        try {
            if (useGzip) {
                // Read from Gzip file
                gzFile inFile = gzopen(fullPath.string().c_str(), "rb");
                if (!inFile) {
                    if (debugMode) wxLogMessage("Failed to open gzip file for reading: %s", fullPath.string().c_str());
                    return false;
                }

                char buffer[128];  // Temporary buffer for reading
                int bytesRead;
                while ((bytesRead = gzread(inFile, buffer, sizeof(buffer) - 1)) > 0) {
                    buffer[bytesRead] = '\0';  // Null-terminate the buffer
                    content += buffer;  // Append read content
                }
                gzclose(inFile);
                if (debugMode) wxLogMessage("Retrieved gzip file: %s", fullPath.string().c_str());
                return true;
            }
            else {
                // Read from regular text file
                std::ifstream inFile(fullPath);
                if (inFile.is_open()) {
                    std::string line;
                    while (std::getline(inFile, line)) {
                        content += line + "\n";  // Read file content
                    }
                    inFile.close();
                    if (debugMode) wxLogMessage("Retrieved file: %s", fullPath.string().c_str());
                    return true;  // Return true on success
                }
                else {
                    if (debugMode) wxLogMessage("Failed to open file for reading: %s", fullPath.string().c_str());
                }
            }
        }
        catch (const std::exception& e) {
            if (debugMode) wxLogMessage("Exception in retrieveFile: %s", e.what());
        }
        return false;  // Return false on failure
    }



};


class FolderRoutine {
    Folder* root;

public:
    static FolderRoutine getFactory() {
        return FolderRoutine();
    }

    FolderRoutine() {
        root = new Folder("main");
        root->loadChildrenFromDisk();
    }

    ~FolderRoutine() {
        for (auto folder : folderManager) {
            delete folder; // Clean up all folders
        }
        folderManager.clear();
    }

    static void makeFolders() {
        Folder* root = new Folder("main");
        Folder* cacheFolder = new Folder("cache");
        Folder* archivesFolder = new Folder("archives");
        root->addChild(cacheFolder);
        root->addChild(archivesFolder);
        root->create(fs::current_path());
        delete root; // Clean up after creating folders
    }

    void testNewMethods() {
        // Test folder creation and file operations
        Folder* root = new Folder("main");
        Folder* cacheFolder = new Folder("cache");
        Folder* archivesFolder = new Folder("archives");
        root->addChild(cacheFolder);
        root->addChild(archivesFolder);
        root->create(fs::current_path());
        
        root->createWithinNamedFolder("cache", "Linkedin");
        root->createDirectoriesFromRelativePath("AB/BC/CD/EF/HI");
        root->storeFile("main/archive/one/two/three/three.txt", "hello how are you");
    }


};

// FolderRoutine class remains the same...


/*class FolderRoutine {
    Folder* root ;
    Folder* folderA ;
    Folder* folderB ;
public:
    static FolderRoutine getFactory() {
        return FolderRoutine();
    }
    FolderRoutine() {
        root = new Folder("main");
        root->loadChildrenFromDisk();
        folderA = new Folder("cache");
        folderB = new Folder("archives");
    }
    ~FolderRoutine() {
        //delete folderB;
        //delete folderA;
        delete root;
    }
    static  void makeFolders() {
        Folder* root = new Folder("main");
        Folder* folderA = new Folder("cache");
        Folder* folderB = new Folder("archives");
        root->addChild(folderA);
        root->addChild(folderB);
        root->create(fs::current_path());
        delete root;
    }

      void testNewMethods() {      
        root->addChild(folderA);
        root->addChild(folderB);

        // Create a folder within "cache"
        root->create(fs::current_path());
        root->createWithinNamedFolder("cache", "Linkedin");

        // Create a folder hierarchy based on a relative path
       // root->createFromFullPath("cache/Linkedin2/Jobs");

        // Save a file within a directory
       // root->saveFile("cache/Linkedin2/Jobs", "file.txt", "This is a test file.");

        // Retrieve the file content
        //std::string fileContent = root->retrieveFile("cache/Linkedin2/Jobs", "file.txt");
        //wxLogMessage("Retrieved file content: %s", fileContent.c_str());

        // Clean up memory
        
       
       
    }
};*/
