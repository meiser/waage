/*
 * File:   main.cpp
 * Author: Stephan Keller
 *
 * Created on February, 2012, 07:40 AM
 */

//#define _WIN32_WINNT 0x0501

#define WAAGE_VERSION "0.1.0"

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/regex.hpp>
#include <tinyxml.h>
#include <string>

#include <map>
#include "SimpleSerial.h"
#include "AsyncSerial.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace pt = boost::posix_time;
using namespace std;


// globale Ordner timestamp_dir and diretory_name, created in main() on startup;

fs::path scale_file_name("last");
fs::path scale_file_directory;
fs::path programm_root;
fs::path config_file_name("config");

// directory for storing all scale files
fs::path data_path("data");

string com_port = "COM1";
int baudrate = 9600;
int databits = 8;
int stopbits = 1;
string parity = "0";

//boost::asio::serial_port_base::parity parity(boost::asio::serial_port_base::parity::none);

bool console_mode=false;


// Callback , speichert Daten lokal zwischen
void received(const char *data, unsigned int len)
{
    vector<char> v(data,data+len);
    fs::ofstream outfile(scale_file_directory/scale_file_name, ios_base::app);
    string line;
    for(unsigned int i=0; i<v.size(); i++)
    {
        // Enfernung von Tabulatoren und Zeilenvorschub
        switch(v[i])
        {

        case '\t':
            break;
        case '\r':
            break;
        case 0x1B:
            cout << "Escape entfernt" << endl;
            break;
        case '\n':
            line += v[i];
            outfile << line;
            line = "";
            break;

        default:
            line += v[i];
        }
    }
    outfile.close();
}

void received_for_console(const char *data, unsigned int len)
{
    vector<char> v(data,data+len);
    for(unsigned int i=0; i<v.size(); i++)
    {
        if(v[i]=='\n')
        {
            cout<<endl;
        }
        else
        {
            if(v[i]<32 || v[i]>=0x7f) cout.put(' ');//Remove non-ascii char
            else cout.put(v[i]);
        }
    }
    cout.flush();//Flush screen buffer
}

int console()
{
    try
    {
        CallbackAsyncSerial serial(com_port, baudrate);
        serial.setCallback(received_for_console);

        string command = "";
        cout << "Bitte Befehle eingeben, Beendigung mit Befehl exit + Enter" << endl;
        while (command != "exit")
        {
            cin >> command;
            if (command != "exit")
            {
                cout << endl;
                cout << "Befehl "+ command+" wird abgeschickt" << endl;
                cout << endl;

                serial.writeString(command);
            }
            else
            {
                cout << "Programm wird beendet und COM Port geschlossen" << endl;
                serial.close();
            }
        }
    }
    catch (std::exception& e)
    {
        cerr<<"Exception: "<<e.what()<<endl;
        return 400;
    }
    return 200;
}


void load_xml_settings(fs::path config_file_path)
{
    fs::ifstream inFile(config_file_path);
    static const boost::regex com_port_regex("^port:\\s*(?<port>\\w+).*$");
    static const boost::regex baudrate_regex("^baudrate:\\s*(?<baudrate>\\d+).*$");
    boost::smatch result;
    while ( inFile )
    {
        std::string current_line;
        std::getline( inFile, current_line );
        if (inFile)
        {
            if (regex_search(current_line, result, com_port_regex))
            {
                com_port = result.str("port");
            }

            if (regex_search(current_line, result, baudrate_regex))
            {
                try
                {
                    baudrate = boost::lexical_cast<int>(result.str("baudrate"));
                }
                catch( boost::bad_lexical_cast const& )
                {
                    std::cout << "Check the syntax of your config file" << std::endl;
                }
            }
        }
    }
}

int main(int ac, char* av[])
{
    try
    {

        // get program root
        programm_root = boost::filesystem::system_complete(av[0]).parent_path();

        // Declare the supported options.
        po::options_description desc("Allowed options");
        desc.add_options()
        ("name,n", po::value<string>(), "name of file and directory for scale file")
        ("help,h", "produce help message")
        ("interactive,i", "activate interactive COM Port mode")
        ("comport,c", po::value<string>()->implicit_value(com_port), "used com port connection. e.g. COM1 or /dev/ttyS0")
        ("baudrate,b", po::value<string>()->implicit_value(boost::lexical_cast<string>(baudrate)), "baudrate for com port connection")
        ("databits,d", po::value<string>()->implicit_value(boost::lexical_cast<string>(databits)), "databits for com port connection")
        ("stopbits,s", po::value<string>()->implicit_value(boost::lexical_cast<string>(stopbits)), "stopbits for com port connection")
        ("parity,p", po::value<string>()->implicit_value(boost::lexical_cast<string>(parity)), "parity for com port connection: [n]o,[o]dd, [e]ven, [m]ark, [s]pace")

        ("file,f", po::value<string>(), "path to config file")
        ;
        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);


        if (vm.count("file"))
        {
            fs::path user_config_file(vm["file"].as<string>());
            load_xml_settings(user_config_file);
        }
        else
        {
            load_xml_settings(programm_root/config_file_name);
        }

        if (vm.count("help"))
        {
            cout << desc << "\n";
            return 200;
        }

        if (vm.count("comport"))
        {
            com_port = vm["comport"].as<string>();
        }

        if (vm.count("baudrate"))
        {
            baudrate = boost::lexical_cast<int>(vm["baudrate"].as<string>());
        }

        if (vm.count("databits"))
        {
            databits = boost::lexical_cast<int>(vm["databits"].as<string>());
        }

        if (vm.count("stopbits"))
        {
            stopbits = boost::lexical_cast<int>(vm["stopbits"].as<string>());
        }

        /*if (vm.count("parity"))
        {

            string given_parity;
            given_parity = vm["parity"].as<string>();


            if (given_parity == "o"){
                //parity = boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::odd);
            }
            if (given_parity == "e"){
                //parity = boost::asio::serial_port_base::parity::even;
            }


            //parity= boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none),

            //cout << parity << endl;
            //switch(boost::lexical_cast<int>(vm["parity"].as<string>())){
                case NO:
                    cout << "no parity" << endl;
                    break;
                case ODD:
                    cout << "ungerade" << endl;
                    break;
                case EVEN:
                    cout << "gerade" << endl;
                    break;
                case MARK:
                    cout << "markiert" << endl;
                    break;
                case SPACE:
                    cout << "Leerzeichen" << endl;
                    break;
                default:
                    cout << "Passt nicht" << endl;
            }
        }
        */

        if (vm.count("interactive"))
        {
            cout << "Virtual console connection to " << com_port << " with :" << endl;
            cout << "Baudrate: " << baudrate << endl;
            cout << "Databits: " << databits << endl;
            cout << "Stopbits: " << stopbits << endl;
            cout << "Parity  : " << parity << endl;

            console();
            return 200;
        }


        if (vm.count("name"))
        {
            scale_file_name = vm["name"].as<string>();
            scale_file_directory = programm_root / data_path / scale_file_name;
        }
        else
        {
            scale_file_directory = programm_root;
        }

        fs::create_directories(scale_file_directory);
        fs::fstream textfile;
        textfile.open(scale_file_directory / scale_file_name, ios_base::out);
        textfile.close();

        cout << "Connection to " << com_port << " with :" << endl;
        cout << "Baudrate: " << baudrate << endl;
        cout << "Databits: " << databits << endl;
        cout << "Stopbits: " << stopbits << endl;
        cout << "Parity  : " << parity << endl;



        try
        {

            SimpleSerial serial(com_port, baudrate);

            fs::ofstream outfile(scale_file_directory/scale_file_name, ios_base::app);
            string line;
            string right_line;
            line = serial.readLine();
            static const boost::regex weight_regex("(?<weight>(([0-9]+\\.[0-9]*)|([0-9]*\\.[0-9]+)|([0-9]+)))");
            boost::smatch result;

            for(std::string::size_type i = 0; i < line.size(); ++i)
            {
                //if(line[i]==0x26) break;
                //if(line[i]==0x26) break;
                //(line[i]<32 || line[i]>=0x7f) break;
                if(line[i]==0x26) break;
                //if(line[i]==0x02) break;
                if(line[i]>=0x7f) break;
                if(line[i]>32 and line[i]!=127)
                {
                    right_line= right_line+line[i];
                }
            }
            outfile << right_line << endl;
            outfile.close();
            cout << right_line << endl;

            float my_val;

            if (regex_search(right_line, result, weight_regex))
            {

                my_val = boost::lexical_cast<double>(result.str("weight"));
                my_val= my_val*1000;
            }

//            return boost::lexical_cast<int>(right_line+"000");



            return boost::lexical_cast<int>(my_val);

        }
        catch(boost::system::system_error& e)
        {
            cout<<"Error: "<<e.what()<<endl;
            return 1;
        }

    }
    catch(boost::system::system_error& e)
    {
        cout<<"Boost System Fehler: "<<e.what()<<endl;
        return 2147483500;
    }
    catch(boost::program_options::unknown_option& e)
    {
        cout << "Parameter falsch: "<<e.what()<<endl;
        cout << "--help zeigt alle Optionen auf" <<endl;
        return 2147483500;
    }
    catch(boost::program_options::invalid_command_line_syntax &e)
    {
        cout<<"Bitte Argument angeben: "<<e.what()<<endl;
        return 2147483500;
    }
    catch(boost::bad_lexical_cast &e)
    {
        cout<<"Typ des Parameter falsch: "<<e.what()<<endl;
        return 2147483500;
    }
}

