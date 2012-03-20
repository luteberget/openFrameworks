#include "xcodeProject.h"
#include <iostream>



/*
 
xcode project files are plists but we can convert to xml, using plutil: 
 
plutil -convert xml1 -o - myproj.xcodeproj/project.pbxproj 
 
as an XML file, it's very odd and fairly unreadable, which is why this code is pretty gnarly. 
 
some additional things that might be useful to try in the future: 
 
(json parsing)  http://emilloer.com/2011/08/15/dealing-with-project-dot-pbxproj-in-ruby/
(objective c) https://github.com/expanz/xcode-editor  
(plist c++) https://github.com/animetrics/PlistCpp
 
*/




// we are going to use POCO for computing the MD5 Hash of file names and paths, etc: 

#include "Poco/HMACEngine.h"
#include "Poco/MD5Engine.h"
using Poco::DigestEngine;
using Poco::HMACEngine;
using Poco::MD5Engine;

// to add things to the xcode project file, we need some template XML around
// these are common things we'll want to add

#define STRINGIFY(A)  #A

//-----------------------------------------------------------------
const char PBXGroup[] = 
STRINGIFY(

    <key>GROUPUUID</key>
    <dict>
        <key>children</key>
        <array>
        </array>
        <key>isa</key>
        <string>PBXGroup</string>
        <key>name</key>
        <string>GROUPNAME</string>
        <key>sourceTree</key>
        <string>&lt;group&gt;</string>      // <group> or SOURCE_ROOT, etc
    </dict>
          
);

//-----------------------------------------------------------------
const char PBXFileReference[] = 
STRINGIFY(

        <key>FILEUUID</key>
        <dict>
            <key>explicitFileType</key>
            <string>FILETYPE</string>
            <key>fileEncoding</key>
            <string>30</string>
            <key>isa</key>
            <string>PBXFileReference</string>
            <key>name</key>
            <string>FILENAME</string>
            <key>path</key>
            <string>FILEPATH</string>
            <key>sourceTree</key>
            <string>SOURCE_ROOT</string>
        </dict>

);

//-----------------------------------------------------------------
const char PBXBuildFile[] = 
STRINGIFY(
          
          <key>BUILDUUID</key>
          <dict>
          <key>fileRef</key>
          <string>FILEUUID</string>
          <key>isa</key>
          <string>PBXBuildFile</string>
          </dict>
          
);

//-----------------------------------------------------------------
const char HeaderSearchPath[] = 
STRINGIFY(
          
    <key>HEADER_SEARCH_PATHS</key>
    <array>
    <string>$(OF_CORE_HEADERS)</string>
    </array>

);


//-----------------------------------------------------------------
const char LDFlags[] = 
STRINGIFY(
          
          <key>OTHER_LDFLAGS</key>
          <array>
          <string>$(OF_CORE_LIBS)</string>
          </array>
          
);

const char workspace[] = 
STRINGIFY(
          
          <?xml version="1.0" encoding="UTF-8"?>
          <Workspace version = "1.0">
              <FileRef
                location = "self:PROJECTNAME.xcodeproj">
              </FileRef>
          </Workspace>
          
);


void xcodeProject::setup(){
	if( target == "osx" ){	
		srcUUID			= "E4B69E1C0A3A1BDC003C02F2";
		addonUUID		= "BB4B014C10F69532006C3DED"; 
		buildPhaseUUID	= "E4B69E200A3A1BDC003C02F2";
	}else{
		srcUUID			= "E4D8936A11527B74007E1F53";
		addonUUID		= "BB16F26B0F2B646B00518274"; 
		buildPhaseUUID	= "E4D8936E11527B74007E1F53";
	}
}


void xcodeProject::saveScheme(){
    ofDirectory dir(projectDir + projectName + ".xcodeproj" + "/xcshareddata/xcschemes/");
    dir.create(true);
    string schemeTo = projectDir  + projectName + ".xcodeproj" + "/xcshareddata/xcschemes/" + projectName + ".xcscheme";
    ofFile::copyFromTo(templatePath + "emptyExample.xcodeproj/xcshareddata/xcschemes/emptyExample.xcscheme", schemeTo);
    cout << "trying to copy " << projectDir + projectName + ".xcodeproj" + "/xcshareddata/xcschemes/emptyExample.xcscheme" << " ------ > " << schemeTo <<endl;
    findandreplaceInTexfile(schemeTo, "emptyExample", projectName);
    string xcsettings = projectDir  + projectName + ".xcodeproj" + "/xcshareddata/WorkspaceSettings.xcsettings";
    ofFile::copyFromTo(templatePath + "emptyExample.xcodeproj/xcshareddata/WorkspaceSettings.xcsettings", xcsettings);
}


void xcodeProject::saveWorkspaceXML(){
    string xcodeProjectWorkspace = projectDir + projectName + ".xcodeproj" + "/project.xcworkspace/contents.xcworkspacedata";
    ofDirectory dir(projectDir + projectName + ".xcodeproj" + "/project.xcworkspace/");
    dir.create(true);
    cout << "trying copy " << templatePath + "emptyExample.xcodeproj/project.xcworkspace/contents.xcworkspacedata" << " -----> " << xcodeProjectWorkspace <<endl;
    ofFile::copyFromTo(templatePath + "/emptyExample.xcodeproj/project.xcworkspace/contents.xcworkspacedata", xcodeProjectWorkspace);
    findandreplaceInTexfile(xcodeProjectWorkspace, "PROJECTNAME", projectName);
}


bool xcodeProject::createProjectFile(){
    
    // todo: some error checking. 
    
    string xcodeProject = ofFilePath::join(projectDir , projectName + ".xcodeproj");
    ofDirectory xcodeDir(xcodeProject);
    xcodeDir.create(true);
    
    
    ofFile::copyFromTo(ofFilePath::join(templatePath,"emptyExample.xcodeproj/project.pbxproj"), 
                       ofFilePath::join(xcodeProject, "project.pbxproj"), true, true);
            
    
    ofFile::copyFromTo(ofFilePath::join(templatePath,"Project.xcconfig"),projectDir, true, true);
    
    if( target == "osx" ){
        ofFile::copyFromTo(ofFilePath::join(templatePath,"openFrameworks-Info.plist"),projectDir, true, true);
    }else{
        ofFile::copyFromTo(ofFilePath::join(templatePath,"ofxiphone-Info.plist"),projectDir, true, true);
        ofFile::copyFromTo(ofFilePath::join(templatePath,"iPhone_Prefix.pch"),projectDir, true, true);
		
		ofDirectory binDirectory(ofFilePath::join(projectDir, "bin"));
		if (!binDirectory.exists()){
			ofDirectory dataDirectory(ofFilePath::join(projectDir, "bin/data"));
			dataDirectory.create(true);							 
		}
		if(binDirectory.exists()){
			ofDirectory dataDirectory(ofFilePath::join(binDirectory.path(), "data"));
			if (!dataDirectory.exists()){
				dataDirectory.create(false);
			}
		}
		ofFile::copyFromTo(ofFilePath::join(templatePath,"bin/data/Default.png"),projectDir + "/bin/data/Default.png", true, true);
		ofFile::copyFromTo(ofFilePath::join(templatePath,"bin/data/Icon.png"),projectDir + "/bin/data/Icon.png", true, true);
    }

    // this is for xcode 4 scheme issues. but I'm not sure this is right. 
    
    saveWorkspaceXML();
    saveScheme();
    
    // make everything relative the right way. 
    string relRoot = getOFRelPath(ofFilePath::removeTrailingSlash(projectDir));
    if (relRoot != "../../../"){
        string relPath2 = relRoot;
        relPath2.erase(relPath2.end()-1);
        findandreplaceInTexfile(projectDir + projectName + ".xcodeproj/project.pbxproj", "../../..", relPath2);
        findandreplaceInTexfile(projectDir + "Project.xcconfig", "../../../", relRoot);
        findandreplaceInTexfile(projectDir + "Project.xcconfig", "../../..", relPath2);
    }
    

    return true;
}



void xcodeProject::renameProject(){
    pugi::xpath_node_set uuidSet = doc.select_nodes("//string[contains(.,'emptyExample')]");
    for (pugi::xpath_node_set::const_iterator it = uuidSet.begin(); it != uuidSet.end(); ++it){
        pugi::xpath_node node = *it;
        string val = it->node().first_child().value();
        findandreplace(val, "emptyExample",  projectName);
        it->node().first_child().set_value(val.c_str());
    }
}


bool xcodeProject::loadProjectFile(){
    string fileName = projectDir + projectName + ".xcodeproj/project.pbxproj";
    renameProject();
    pugi::xml_parse_result result = doc.load_file(ofToDataPath(fileName).c_str());
    
    return result.status==pugi::status_ok;
    
}  



bool xcodeProject::saveProjectFile(){
    
    
    
    // does this belong here?
    
    renameProject();
    
    // save the project out:
    
    string fileName = projectDir + projectName + ".xcodeproj/project.pbxproj";
    bool bOk =  doc.save_file(ofToDataPath(fileName).c_str());
    return bOk;
}  


bool xcodeProject::findArrayForUUID(string UUID, pugi::xml_node & nodeMe){
    char query[255];
    sprintf(query, "//string[contains(.,'%s')]", UUID.c_str());
    pugi::xpath_node_set uuidSet = doc.select_nodes(query);
    for (pugi::xpath_node_set::const_iterator it = uuidSet.begin(); it != uuidSet.end(); ++it){
        pugi::xpath_node node = *it;
        if (strcmp(node.node().parent().name(), "array") == 0){
            nodeMe = node.node().parent();
            return true;
        } else {
        }
    }
    return false;
}



string generateUUID(string input){
    
    std::string passphrase("openFrameworks"); // HMAC needs a passphrase
    
    HMACEngine<MD5Engine> hmac(passphrase); // we'll compute a MD5 Hash
    hmac.update(input);
    
    const DigestEngine::Digest& digest = hmac.digest(); // finish HMAC computation and obtain digest
    std::string digestString(DigestEngine::digestToHex(digest)); // convert to a string of hexadecimal numbers
    
    return digestString;	
    
    
}





pugi::xml_node xcodeProject::findOrMakeFolderSet(pugi::xml_node nodeToAddTo, vector < string > & folders, string pathForHash){
    
    
    
    
    char query[255];
    sprintf(query, "//key[contains(.,'%s')]/following-sibling::node()[1]//array/string", nodeToAddTo.previous_sibling().first_child().value());
    pugi::xpath_node_set array = doc.select_nodes(query);
    
    bool bAnyNodeWithThisName = false;
    pugi::xml_node nodeWithThisName;
    string name = folders[0];
    
    
    for (pugi::xpath_node_set::const_iterator it = array.begin(); it != array.end(); ++it){
        
        pugi::xpath_node node = *it;
        //node.node().first_child().print(std::cout);
        
        // this long thing checks, is this a pbxgroup, and if so, what's it's name. 
        // do it once for path and once for name, since ROOT level pbxgroups have a path name. ugh. 
        
        char querypbx[255];
        sprintf(querypbx, "//key[contains(.,'%s')]/following-sibling::node()[1]//string[contains(.,'PBXGroup')]/parent::node()[1]//key[contains(.,'path')]/following-sibling::node()[1]", node.node().first_child().value());
        if (doc.select_single_node(querypbx).node() != NULL){
            
            if (strcmp(doc.select_single_node(querypbx).node().first_child().value(), folders[0].c_str()) == 0){
                printf("found matching node \n");
                bAnyNodeWithThisName = true;
                nodeWithThisName = doc.select_single_node(querypbx).node().parent();
            }
        }
        
        sprintf(querypbx, "//key[contains(.,'%s')]/following-sibling::node()[1]//string[contains(.,'PBXGroup')]/parent::node()[1]//key[contains(.,'name')]/following-sibling::node()[1]", node.node().first_child().value());
        
        if (doc.select_single_node(querypbx).node() != NULL){
            if (strcmp(doc.select_single_node(querypbx).node().first_child().value(), folders[0].c_str()) == 0){
                bAnyNodeWithThisName = true;
                nodeWithThisName = doc.select_single_node(querypbx).node().parent();
            }
        }
        
    }
    
   
    
    // now, if we have a pbxgroup with the right name, pop this name off the folder set, and keep going. 
    // else, let's add a folder set, boom. 
    
    if (bAnyNodeWithThisName == false){
        
        // make a new UUID 
        // todo get the full path here somehow... 
        
        pathForHash += "/" + folders[0];
        
        string UUID = generateUUID(pathForHash);
        
        // add a new node
        string PBXGroupStr = string(PBXGroup);
        findandreplace( PBXGroupStr, "GROUPUUID", UUID);
        findandreplace( PBXGroupStr, "GROUPNAME", folders[0]);
        
        pugi::xml_document pbxDoc;
        pugi::xml_parse_result result = pbxDoc.load_buffer(PBXGroupStr.c_str(), strlen(PBXGroupStr.c_str()));
        
        
        nodeWithThisName = doc.select_single_node("/plist[1]/dict[1]/dict[2]").node().prepend_copy(pbxDoc.first_child().next_sibling()); 
        doc.select_single_node("/plist[1]/dict[1]/dict[2]").node().prepend_copy(pbxDoc.first_child()); 
        
        
        
        // add to array
        char queryArray[255];
        sprintf(queryArray, "//key[contains(.,'%s')]/following-sibling::node()[1]//array", nodeToAddTo.previous_sibling().first_child().value());
        doc.select_single_node(queryArray).node().append_child("string").append_child(pugi::node_pcdata).set_value(UUID.c_str());
        //array.begin()->node().parent().append_child("string").append_child(pugi::node_pcdata).set_value(UUID.c_str());
        
        
    } else {
        
        pathForHash += "/" + folders[0];
    }
    
    
    folders.erase(folders.begin());
    
    if (folders.size() > 0){ 
        return findOrMakeFolderSet(nodeWithThisName, folders, pathForHash);
    } else {
        return nodeWithThisName;
    }

}



void xcodeProject::addSrc(string srcFile, string folder){
    
	
    string buildUUID;
    
    //-----------------------------------------------------------------
    // find the extension for the file that's passed in. 
    //-----------------------------------------------------------------
    
    size_t found = srcFile.find_last_of(".");
    string ext = srcFile.substr(found+1);
    
    //-----------------------------------------------------------------
    // based on the extension make some choices about what to do:
    //-----------------------------------------------------------------
    
    bool addToResources = true;
    bool addToBuild = true;
    string fileKind = "file";
    bool bAddFolder = true;
    
    if( ext == "cpp" || ext == "cc"){
        fileKind = "sourcecode.cpp.cpp";
        addToResources = false;													
    }
    if( ext == "c" ){
        fileKind = "sourcecode.c.c";				
        addToResources = false;															
    }
    if(ext == "h" || ext == "hpp"){
        fileKind = "sourcecode.c.h";
        addToBuild = false;
        addToResources = false;													
    }
    if(ext == "mm" || ext == "m"){
        addToResources = false;						
        fileKind = "sourcecode.cpp.objcpp";
    }
    if(ext == "xib"){
		fileKind = "file";
        addToBuild	= true;
    }
    if (folder == "src"){
        bAddFolder = false;
    }
    
    //-----------------------------------------------------------------
    // (A) make a FILE REF
    //-----------------------------------------------------------------
    
    string pbxfileref = string(PBXFileReference);
    string UUID = generateUUID(srcFile);   // replace with theo's smarter system. 
    
    string name, path;
    splitFromLast(srcFile, "/", path, name);
    
    findandreplace( pbxfileref, "FILENAME", name);
    findandreplace( pbxfileref, "FILEPATH", srcFile);
    findandreplace( pbxfileref, "FILETYPE", fileKind);
    findandreplace( pbxfileref, "FILEUUID", UUID);
    
    pugi::xml_document fileRefDoc;
    pugi::xml_parse_result result = fileRefDoc.load_buffer(pbxfileref.c_str(), strlen(pbxfileref.c_str()));
    
    // insert it at <plist><dict><dict>
    doc.select_single_node("/plist[1]/dict[1]/dict[2]").node().prepend_copy(fileRefDoc.first_child().next_sibling());   // UUID FIRST
    doc.select_single_node("/plist[1]/dict[1]/dict[2]").node().prepend_copy(fileRefDoc.first_child());                  // DICT SECOND
    
    //-----------------------------------------------------------------
    // (B) BUILD REF
    //-----------------------------------------------------------------
    
    if (addToBuild == true){
        
        buildUUID = generateUUID(srcFile + "-build");
        string pbxbuildfile = string(PBXBuildFile);
        findandreplace( pbxbuildfile, "FILEUUID", UUID);
        findandreplace( pbxbuildfile, "BUILDUUID", buildUUID);
        fileRefDoc.load_buffer(pbxbuildfile.c_str(), strlen(pbxbuildfile.c_str()));
        doc.select_single_node("/plist[1]/dict[1]/dict[2]").node().prepend_copy(fileRefDoc.first_child().next_sibling());   // UUID FIRST
        doc.select_single_node("/plist[1]/dict[1]/dict[2]").node().prepend_copy(fileRefDoc.first_child());                  // DICT SECOND
        
        // add it to the build array. 
        pugi::xml_node array;
        findArrayForUUID(buildPhaseUUID, array);    // this is the build array (all build refs get added here)
        array.append_child("string").append_child(pugi::node_pcdata).set_value(buildUUID.c_str());
    }
    
    //-----------------------------------------------------------------
    // (C) resrouces 
    //-----------------------------------------------------------------
    
    
    if (addToResources == true){
        // no idea where resources go
    }
    
    
    //-----------------------------------------------------------------
    // (D) folder
    //-----------------------------------------------------------------
    
 
    if (bAddFolder == true){
        
        vector < string > folders = ofSplitString(folder, "/", true);
        
        if (folders.size() > 1){
            if (folders[0] == "src"){
                string xmlStr = "//key[contains(.,'"+srcUUID+"')]/following-sibling::node()[1]";
				
                folders.erase(folders.begin());
                pugi::xml_node node = doc.select_single_node(xmlStr.c_str()).node();
                pugi::xml_node nodeToAddTo = findOrMakeFolderSet( node, folders, "src");
                nodeToAddTo.child("array").append_child("string").append_child(pugi::node_pcdata).set_value(UUID.c_str());
                
            } else if (folders[0] == "addons"){
                string xmlStr = "//key[contains(.,'"+addonUUID+"')]/following-sibling::node()[1]";

                folders.erase(folders.begin());
                pugi::xml_node node = doc.select_single_node(xmlStr.c_str()).node();
                pugi::xml_node nodeToAddTo = findOrMakeFolderSet( node, folders, "addons");
                
                nodeToAddTo.child("array").append_child("string").append_child(pugi::node_pcdata).set_value(UUID.c_str());
                
            } else {
                string xmlStr = "//key[contains(.,'"+srcUUID+"')]/following-sibling::node()[1]";

                pugi::xml_node node = doc.select_single_node(xmlStr.c_str()).node();
                
                // I'm not sure the best way to proceed;
                // we should maybe find the rootest level and add it there. 
                // TODO: fix this. 
            }
        }; 
        
    } else {
        
        
        pugi::xml_node array;
		string xmlStr = "//key[contains(.,'"+srcUUID+"')]/following-sibling::node()[1]";
        pugi::xml_node node = doc.select_single_node(xmlStr.c_str()).node();
        node.child("array").append_child("string").append_child(pugi::node_pcdata).set_value(UUID.c_str());
        //nodeToAddTo.child("array").append_child("string").append_child(pugi::node_pcdata).set_value(UUID.c_str());
        
    }
        
    //saveFile(projectDir + "/" + projectName + ".xcodeproj" + "/project.pbxproj");
} 


// todo: these three have very duplicate code... please fix up a bit. 

void xcodeProject::addInclude(string includeName){
    
    
    

    char query[255];
    sprintf(query, "//key[contains(.,'baseConfigurationReference')]/parent::node()//key[contains(.,'HEADER_SEARCH_PATHS')]/following-sibling::node()[1]");
    pugi::xpath_node_set headerArray = doc.select_nodes(query);

    if (headerArray.size() > 0){
        
        for (pugi::xpath_node_set::const_iterator it = headerArray.begin(); it != headerArray.end(); ++it){
            pugi::xpath_node node = *it;
            //node.node().print(std::cout);
            node.node().append_child("string").append_child(pugi::node_pcdata).set_value(includeName.c_str());
        }
        
    } else {
        
        //printf("we don't have HEADER_SEARCH_PATHS, so we're adding them... and calling this function again \n");
        query[255];
        sprintf(query, "//key[contains(.,'baseConfigurationReference')]/parent::node()//key[contains(.,'buildSettings')]/following-sibling::node()[1]");
        pugi::xpath_node_set dictArray = doc.select_nodes(query);
        
        
        for (pugi::xpath_node_set::const_iterator it = dictArray.begin(); it != dictArray.end(); ++it){
            pugi::xpath_node node = *it;
            
            //node.node().print(std::cout);
            string headerXML = string(HeaderSearchPath);
            pugi::xml_document headerDoc;
            pugi::xml_parse_result result = headerDoc.load_buffer(headerXML.c_str(), strlen(headerXML.c_str()));
            
            // insert it at <plist><dict><dict>
            node.node().prepend_copy(headerDoc.first_child().next_sibling());   // KEY FIRST
            node.node().prepend_copy(headerDoc.first_child());                  // ARRAY SECOND
            
        }
        
        // now that we have it, try again...
        addInclude(includeName);
    }
    
    //saveFile(projectDir + "/" + projectName + ".xcodeproj" + "/project.pbxproj");

}  
        
        
void xcodeProject::addLibrary(string libraryName){
    
    cout << " adding libraryName " << libraryName << endl; 
    
    
    char query[255];
    sprintf(query, "//key[contains(.,'baseConfigurationReference')]/parent::node()//key[contains(.,'OTHER_LDFLAGS')]/following-sibling::node()[1]");
    pugi::xpath_node_set headerArray = doc.select_nodes(query);
    
    
    if (headerArray.size() > 0){
        for (pugi::xpath_node_set::const_iterator it = headerArray.begin(); it != headerArray.end(); ++it){
            pugi::xpath_node node = *it;
            node.node().append_child("string").append_child(pugi::node_pcdata).set_value(libraryName.c_str());
        }
        
    } else {
        
        //printf("we don't have OTHER_LDFLAGS, so we're adding them... and calling this function again \n");
        query[255];
        sprintf(query, "//key[contains(.,'baseConfigurationReference')]/parent::node()//key[contains(.,'buildSettings')]/following-sibling::node()[1]");
        
        pugi::xpath_node_set dictArray = doc.select_nodes(query);

        for (pugi::xpath_node_set::const_iterator it = dictArray.begin(); it != dictArray.end(); ++it){
            pugi::xpath_node node = *it;
            
            //node.node().print(std::cout);
            string ldXML = string(LDFlags);
            pugi::xml_document ldDoc;
            pugi::xml_parse_result result = ldDoc.load_buffer(ldXML.c_str(), strlen(ldXML.c_str()));
            
            // insert it at <plist><dict><dict>
            node.node().prepend_copy(ldDoc.first_child().next_sibling());   // KEY FIRST
            node.node().prepend_copy(ldDoc.first_child());                  // ARRAY SECOND
            
            //node.node().print(std::cout);
        }
        
        // now that we have it, try again...
        addLibrary(libraryName);
    }
    
    //saveFile(projectDir + "/" + projectName + ".xcodeproj" + "/project.pbxproj");
}
