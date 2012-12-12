/*
 * File:   main.cpp
 * Author: Stephan Keller
 *
 * Created on February, 2012, 07:40 AM
 */

//#define _WIN32_WINNT 0x0501

#define WAAGE_VERSION "0.1.0"

#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/regex.hpp>
#include <string>
#include <map>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace pt = boost::posix_time;
using namespace std;
using boost::asio::ip::tcp;


// globale Ordner timestamp_dir and diretory_name, created in main() on startup;

fs::path scale_file_name("last");
fs::path scale_file_directory;
fs::path programm_root;
fs::path config_file_name("config");

// directory for storing all scale files
fs::path data_path("data");

string host= "127.0.0.1";
string port= "8000";


void load_xml_settings(fs::path config_file_path)
{
    fs::ifstream inFile(config_file_path);
    static const boost::regex port_regex("^port:\\s*(?<port>\\d+).*$");
    static const boost::regex host_regex("^host:\\s*(?<host>\\S+).*$");
    //^port:\s*(?<port>\d*)
    boost::smatch result;
    while ( inFile )
    {
        std::string current_line;
        std::getline( inFile, current_line );
        if (inFile)
        {
            if (regex_search(current_line, result, port_regex))
            {
                port = result.str("port");
            }

            if (regex_search(current_line, result, host_regex))
            {
                host = result.str("host");
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
        ("server,s", po::value<string>()->implicit_value(host), "used host for connection. e.g. 127.0.0.1 or localhost")
        ("port,p", po::value<string>()->implicit_value(port), "used port for connection. e.g. 8000")
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
            port = vm["port"].as<string>();
        }

        if (vm.count("host"))
        {
            port = vm["host"].as<string>();
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

        cout << "Connection to " << host << " on port: " <<  port << endl;

        try
        {

            boost::asio::io_service io_service;

            tcp::resolver resolver(io_service);
            tcp::resolver::query query(host, port);
            tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
            tcp::resolver::iterator end;

            tcp::socket socket(io_service);
            std::string message("<FP>");
            boost::system::error_code error = boost::asio::error::host_not_found;

            while (error && endpoint_iterator != end)
            {
                socket.close();
                socket.connect(*endpoint_iterator++, error);
            }
            if (error)
                throw boost::system::system_error(error);


            boost::asio::write(socket, boost::asio::buffer(message,sizeof(message)));//,boost::asio::transfer_all(),IgnoreError);

            boost::array<char, 50> buf;
            socket.read_some(boost::asio::buffer(buf), error);

            message = buf.data();

            static const boost::regex weight_regex("(\\d+)(?!.*\\d)");
            boost::smatch result;

            int weight;

            if (regex_search(message, result, weight_regex))
            {
                weight = boost::lexical_cast<int>(result);
            }
            else
            {
                weight = 0;
            }

            fs::ofstream outfile(scale_file_directory/scale_file_name, ios_base::app);
            outfile << buf.data();
            outfile.close();

            weight= weight*1000;
            cout << (weight);
            return weight;
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

