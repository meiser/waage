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


#include <map>
#include "AsyncSerial.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace pt = boost::posix_time;
using namespace std;


// globale Ordner timestamp_dir and diretory_name, created in main() on startup;

fs::path scale_file_name;
fs::path scale_file_directory;
fs::path programm_root;
fs::path config_file_name("config");

// directory for storing all scale files
fs::path data_path("data");

string fmp_com_port = "COM1";
int fmp_baudrate = 9600;

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
        CallbackAsyncSerial serial(fmp_com_port, fmp_baudrate);
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
                fmp_com_port = result.str("port");
            }

            if (regex_search(current_line, result, baudrate_regex))
            {
                try
                {
                    fmp_baudrate = boost::lexical_cast<int>(result.str("baudrate"));
                }
                catch( boost::bad_lexical_cast const& )
                {
                    std::cout << "Check the syntax of your config file" << std::endl;
                }
            }
        }
    }
}




void my_thread(fs::path full_path)
{
    boost::this_thread::sleep(pt::milliseconds(10000));
    fs::path test = "MIKE1234";
    fs::create_directories(scale_file_directory/test);
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
        ("console,c", po::value<string>()->implicit_value(""), "interactive COM Port mode")
        ("port,p", po::value<string>()->implicit_value(fmp_com_port), "used com port connection. e.g. COM1 or /dev/ttyS0")
        ("baudrate,b", po::value<string>()->implicit_value(boost::lexical_cast<string>(fmp_baudrate)), "baudrate for com port connection")
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

        if (vm.count("port"))
        {
            fmp_com_port = vm["port"].as<string>();
        }

        if (vm.count("baudrate"))
        {
            fmp_baudrate = boost::lexical_cast<int>(vm["baudrate"].as<string>());
        }

        if (vm.count("console"))
        {
            cout << "Virtual console connection to " << fmp_com_port << " with " << fmp_baudrate << endl;
            console();
            return 200;
        }

        if (vm.count("name"))
        {
            scale_file_name = vm["name"].as<string>();

            scale_file_directory = programm_root / data_path / scale_file_name;
            fs::create_directories(scale_file_directory);
            fs::fstream textfile;
            textfile.open(scale_file_directory / scale_file_name, ios_base::out);
            textfile.close();
        }

        cout << "Connection to " << fmp_com_port << " with " << fmp_baudrate << endl;

        CallbackAsyncSerial serial(fmp_com_port, fmp_baudrate);
        serial.setCallback(received);
        serial.writeString("<FP>");
        int first;
        int second;
        bool still_changing = true;
        while (still_changing)
        {
            first = fs::file_size(scale_file_directory / scale_file_name);
            boost::this_thread::sleep(pt::milliseconds(1000));
            second = fs::file_size(scale_file_directory / scale_file_name);
            if (second == first)
            {
                still_changing = false;
            }
        }

        // Nummer Waage, wird intern erzeugt
        static const boost::regex scale_number("^\\s*Nr.\\s*(?<nummer>\\d+).*$");
        // Bereich
        static const boost::regex scale_bereich("^\\s*Bereich\\s*(?<bereich>\\d+).*$");
        // Brutto
        static const boost::regex scale_brutto("^\\s*Brutto\\s*(?<brutto>\\d+,?\\d*)\\s*(?<einheit>\\S+).*$");
        //Tara
        static const boost::regex scale_tara("^\\s*Tara\\s*(?<tara>\\d+,?\\d*).*$");
        //Netto
        static const boost::regex scale_netto("^\\s*Netto\\s*(?<netto>\\d+,?\\d*).*$");

        string current_line;
        boost::smatch result;

        map<string,string> scale_values;

        fs::ifstream fin(scale_file_directory / scale_file_name);
        while(getline(fin, current_line))
        {
            // Ende des Blocks erreicht, Aufbau der XML

            if (regex_search(current_line, result, scale_number))
            {
                scale_values["nummer"] = result.str("nummer");
            }

            if (regex_search(current_line, result, scale_bereich))
            {
                scale_values["bereich"] = result.str("bereich");
            }

            if (regex_search(current_line, result, scale_brutto))
            {
                scale_values["brutto"] = result.str("brutto");
                scale_values["einheit"] = result.str("einheit");
            }

            if (regex_search(current_line, result, scale_tara))
            {
                scale_values["tara"] = result.str("tara");
            }

            if (regex_search(current_line, result, scale_netto))
            {
                scale_values["netto"] = result.str("netto");
            }
        }

        map<string, string>::iterator iter = scale_values.find("netto");

        if ( scale_values.end() != iter ) {
            cout << "Es gibt Netto: " << scale_values["netto"] << endl;
        }
        else
        {
            cout << "Es gibt nur Brutto: " << scale_values["brutto"] << endl;
        }

        //cout << scale_values["netto"] << endl;


        return 1;//boost::lexical_cast<int>(scale_values["netto"]);
    }
    catch(boost::system::system_error& e)
    {
        cout<<"Boost System Fehler: "<<e.what()<<endl;
        return 400;
    }
    catch(boost::program_options::unknown_option& e)
    {
        cout << "Parameter falsch: "<<e.what()<<endl;
        cout << "--help zeigt alle Optionen auf" <<endl;
        return 400;
    }
    catch(boost::program_options::invalid_command_line_syntax &e)
    {
        cout<<"Bitte Argument angeben: "<<e.what()<<endl;
        return 400;
    }
}

