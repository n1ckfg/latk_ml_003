#ifndef PROPERTIES_FILE_READER_H
#define PROPERTIES_FILE_READER_H

#include <string>
#include <map>
#include <fstream>

using namespace std;

/**\brief Properties map factory
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria 
 * \details reads a properties file and creates a key=value string map*/
class PropertiesFileReader {
    public:
        PropertiesFileReader ()  {}
        
        /**\brief read a file and parse it for properties
         * \param strFile path to file
         * \return true if reading terminated successfully*/
        bool Read (const string& strFile) {
            ifstream is(strFile.c_str());
            if(is.fail())
                return false;
            if (!is.is_open()) 
                return false;
            while (!is.eof()) {
                string strLine;
                getline(is,strLine);
                int nPos = strLine.find('=');
                if (string::npos == nPos) 
                    continue; // no '=', invalid line;
                string strKey = strLine.substr(0,nPos);
                string strVal = strLine.substr(nPos + 1, strLine.length() - nPos + 1);
                m_map.insert(map<string,string>::value_type(strKey,strVal));
            }
            filename=strFile;
            return true;
        }
        
        /**\brief read a file again
         * \return true if successfull*/
        bool reload(){
            m_map.clear();
            return Read(filename);
        }

        /**\brief access property string value
         * \param strkey property name (key)
         * \return string property value if exists, or "" otherwise */
        string V(const string& strKey) const {
            string strValue = "";
            map<string,string>::const_iterator i;
            i = m_map.find(strKey);
            if (i != m_map.end()) {
                strValue = i->second;
            }
            return strValue;
        }

    protected:
        map<string,string> m_map;
        string filename;
}; 

#endif