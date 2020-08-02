#include <string>
#include "core/config_parser.h"
#include "core/config.pb.h"



class ConfigParserTestCase{
    public:
        virtual bool run(){
            auto config_parser = ConfigParser<Config>();
            std::string config_fn = "./default.cfg";
            config_parser.SaveToTxt(config_fn);
            config_parser.LoadFromTxt(config_fn);
            config_parser.Print();
            return true;
        }
};



int main(){
    return 0;
}

