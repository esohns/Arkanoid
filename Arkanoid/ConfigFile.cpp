//
//  ConfigFile.cpp
//  Arkanoid
//
//  Created by Maciej Żurad on 11/25/12.
//  Copyright (c) 2012 Maciej Żurad. All rights reserved.
//

#include "ConfigFile.h"


ConfigFile::ConfigFile(string filename){
    ConfigFile::filename = filename;
    Parse();
}


template <typename T>
T ConfigFile::String_to_T(string const &val){
    
    std::istringstream istr(val);
    T returnVal;
    
    if (!(istr >> returnVal))        
        ExitWithError("ConfigFile: Not a valid " + (std::string)typeid(T).name() + " received!\n");
        
        return returnVal;
}

void ConfigFile::ExitWithError(const string &error){
    
	cerr << error;
	cin.ignore();
	cin.get();
    
	exit(EXIT_FAILURE);
}



void ConfigFile::RemoveComment(string &line) const{
    if(line.find(";") != line.npos)
        line.erase(line.find(";"));
}

bool ConfigFile::HasOnlyWhiteSpace(string &line) const{
    return (line.find_first_not_of(' ') == line.npos);
}

bool ConfigFile::IsValidLine(string &line) const{
    
    line.erase(0,line.find_first_not_of("\t "));
    if(line[0] == '=')
        return false;
    //whitespaces counter
    int ws_counter = 0;
    
    //removing spaces
    line.erase( remove( line.begin(), line.end(), ' ' ), line.end() );
    
    //counting comma's
    for(size_t i=0; i<line.length(); i++)
        if(line[i] == ',')
            ws_counter++;
    
    if(ws_counter == 9)
        return true;

    return false;
    
}

bool ConfigFile::KeyExists(char key) const{
    return content.find(key) != content.end();
}

void ConfigFile::ExtractKey(char &key, string &line) const{
    key = line[0];
    line.erase(0,2);
}

void ConfigFile::ExtractValue(Value &value, string &line) const{
    
    size_t it = 0;
    
    value.filename = line.substr(0, it=line.find_first_of(','));
    line.erase(0, it+1);
    value.maxFrame = String_to_T<int>(line.substr(0, it=line.find_first_of(',')));
    line.erase(0, it+1);
    value.frameDelay = String_to_T<int>(line.substr(0, it=line.find_first_of(',')));
    line.erase(0, it+1);
    value.frameWidth = String_to_T<int>(line.substr(0, it=line.find_first_of(',')));
    line.erase(0, it+1);
    value.frameHeight = String_to_T<int>(line.substr(0, it=line.find_first_of(',')));
    line.erase(0, it+1);
    value.animationColumns = String_to_T<int>(line.substr(0, it=line.find_first_of(',')));
    line.erase(0, it+1);
    value.animationDirection = String_to_T<int>(line.substr(0, it=line.find_first_of(',')));
    line.erase(0, it+1);
    value.speed = String_to_T<int>(line.substr(0, it=line.find_first_of(',')));
    line.erase(0, it+1);
    value.dirX = String_to_T<int>(line.substr(0, it=line.find_first_of(',')));
    line.erase(0, it+1);
    value.health = String_to_T<int>(line.substr(0, it=line.find_first_of(',')));
    
}

void ConfigFile::ExtractContents(string &line, size_t line_number){

    char key;
    Value value;
    
    ExtractKey(key, line);
    ExtractValue(value, line);
    
    if(!KeyExists(key))
        content.insert(pair<char, Value>(key, value));
    else
        ExitWithError("ConfigFile: Only unique keys are allowed at " + line_number);
    
}

void ConfigFile::Parse(){
    ifstream file;
    file.open(filename.c_str());
    
    if(!file)
        ExitWithError("ConfigFile: File" + filename + "couldn't be found!\n");
    
    string line ;
    size_t line_number = 0;
    while(getline(file, line)){
        ++line_number;
        
        RemoveComment(line);
        if(HasOnlyWhiteSpace(line)) // empty line so we continue
            continue;
        if(!IsValidLine(line))
            ExitWithError("ConfigFile: Found invalid line at " + line_number);
        ExtractContents(line, line_number);
    }

    file.close();
}
