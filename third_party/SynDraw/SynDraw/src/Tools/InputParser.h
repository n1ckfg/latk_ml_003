#ifndef INPUTPARSER_CPP
#define INPUTPARSER_CPP

#include <vector>
#include <algorithm>

/**\brief Parser for command line arguments
 * \author Bastien Wailly <bastien.wailly@inria.fr>, Inria */
class InputParser{
    public:
        InputParser(){};
        
        /**\brief create a parser on argc and argv*/
        InputParser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(std::string(argv[i]));
        }

        /**\brief get the value of an command option argument 
         * \param option the flag we are looking for (ex: -p)
         * \return the string value of the argument
         * \details for example in $> ./exe -p abc, the value of -p is "abc" */
        const std::string& getCmdOption(const std::string &option) const{
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            static const std::string empty_string("");
            return empty_string;
        }

        /**\brief check if argument flag exists
         * \param option the argument to check
         * \return true if command exists in arguments*/
        bool cmdOptionExists(const std::string &option) const{
            return std::find(this->tokens.begin(), this->tokens.end(), option)
                   != this->tokens.end();
        }
    private:
        std::vector <std::string> tokens;
};


#endif /* INPUTPARSER_CPP */

