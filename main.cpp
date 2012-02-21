/*
 * File:   main.cpp
 * Author: Stephan Keller
 *
 * Created on February, 2012, 07:40 AM
 */

//#define _WIN32_WINNT 0x0501

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/regex.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <tinyxml.h>

#include "AsyncSerial.h"




namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace pt = boost::posix_time;
using namespace std;


// globale Ordner timestamp_dir and diretory_name, created in main() on startup;

fs::path timestamp_dir;
fs::path directory_name;
fs::path programm_root;
fs::path config_file_name("config");
fs::path data_path("data");

string fmp_com_port = "COM1";
int fmp_baudrate = 9600;


bool console_mode=false;




// Callback , speichert Daten lokal zwischen
void received(const char *data, unsigned int len)
{
    vector<char> v(data,data+len);
    fs::ofstream outfile(timestamp_dir/directory_name, ios_base::app);
    for(unsigned int i=0; i<v.size(); i++)
    {
        // Enfernung von Tabulatoren und Zeilenvorschub
        switch(v[i])
        {

        case '\t':
            break;
        case '\r':
            break;
        default:
            outfile << v[i];
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

                serial.writeString(command+"\r\n");
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

int main(int ac, char* av[])
{
    try
    {

        // get program root
        programm_root = boost::filesystem::system_complete(av[0]).parent_path();

        // Declare the supported options.
        po::options_description desc("Allowed options");
        desc.add_options()
        ("help,h", "produce help message")
        ("start_time,s", po::value<string>()->implicit_value(""), "start time")
        ("console,c", po::value<string>()->implicit_value(""), "interactive COM Port mode")
        ("input,i", po::value<string>()->implicit_value(fmp_com_port), "used com port connection. e.g. COM1 or /dev/ttyS0")
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

        if (vm.count("console"))
        {
            console();
            return 200;
        }


        if (vm.count("input"))
        {
            fmp_com_port = vm["input"].as<string>();
        }

        if (vm.count("baudrate"))
        {
            fmp_baudrate = boost::lexical_cast<int>(vm["baudrate"].as<string>());
        }

        if (vm.count("start_time"))
        {
            directory_name = vm["start_time"].as<string>();
            data_path = "data";
            timestamp_dir = programm_root / data_path / directory_name;
            fs::create_directories(timestamp_dir);
            fs::fstream textfile;
            textfile.open(timestamp_dir / directory_name, ios_base::out);
            textfile.close();
        }
        else
        {
            cout << "Zeitstring muss angegeben werden: --start_time <int> \n";
            return 400;
        }

        cout << "Connection to " << fmp_com_port << " with " << fmp_baudrate << endl;

        CallbackAsyncSerial serial(fmp_com_port, fmp_baudrate);
        serial.setCallback(received);
        serial.writeString("SAM\r\n");
        int first;
        int second;
        bool still_changing = true;
        while (still_changing)
        {
            first = fs::file_size(timestamp_dir / directory_name);
            boost::this_thread::sleep(pt::milliseconds(1000));
            second = fs::file_size(timestamp_dir / directory_name);
            if (second == first)
            {
                still_changing = false;
            }
        }

        //get file separator

        //Regex für Findung Auftragsnummer und Kommentar
        static const boost::regex block_comment("^Block-Kommentar:(?<orno>\\d+)?\\s*(?<comment>.*)$");
        //Regex für Findung Blockseparator, zeigt Beginn eines neuen Blocks an, im Geraet sind moeglich
        //GS Hex code 0x1d (none printable characters ASCII, http://web.cs.mun.ca/~michael/c/ascii-table.html)
        //,
        //*
        //;
        //#
        //:
        static const boost::regex separator("^(?<sep>[,*;#:]|\x1d)$");
        //Regex für Findung der Messwerte
        static const boost::regex measure_value("^(?<value>[-+]?\\d+\\.\\d+)$");
        //Regex für Applikationsname
        static const boost::regex application_regex("^Appl-Name:(?<app>.+)$");
        //Regex für Datum
        //static const boost::regex date_regex("^Datum:(?<date>.+)$");

        static const boost::regex date_regex("^Datum:0?(?<day>\\d+)\\.0?(?<month>\\d*)\\.(?<year>\\d+)$");
        //Regex für Zeit
        //static const boost::regex time_regex("^Zeit:(?<time>.+)$");
        static const boost::regex time_regex("^Zeit:0?(?<hour>\\d+):0?(?<minute>\\d+)$");


        //Regex für obere Toleranzgrenze und Einheit
        static const boost::regex max_value_regex("^obere Toleranzgr.:(?<max>[-+]?\\d+)(?<unit>.+)$");
        //Regex für obere Toleranzgrenze
        static const boost::regex min_value_regex("^untere Toleranzgr.:(?<min>[-+]?\\d+)(?<unit>.+)$");

        //Applikationsnamen
        string application_name;
        //Blockkommentar
        string comment;
        //Ornonumber
        string orno_number;
        //Blockdatum zerlegt
        string block_day;
        string block_month;
        string block_year;
        //Blockzeit zerlegt
        string block_hour;
        string block_minute;
        //obere Grenze
        string border_max;
        //untere Grenze
        string border_min;
        //Einheit
        string unit;
        //Blockanzahl
        int amount_blocks = 0;
        //Messwerte abgespeichert in Stringliste
        list<string> values;

        //Kopf XML-Datei
        TiXmlDocument doc;
        TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "utf-8", "yes" );
        TiXmlElement * application = new TiXmlElement( "application" );

        cout << "Beginn Erstellung XML Datei..." << endl;

        string current_line;
        //Ergebnis Regex
        boost::smatch result;
        //Datei
        fs::ifstream fin(timestamp_dir / directory_name);

        //Durchsuchung Datei pro Zeile und Extrahierung der Blockdaten
        while(getline(fin, current_line))
        {
            // Ende des Blocks erreicht, Aufbau der XML

            if (regex_search(current_line, result, separator))
            {
                amount_blocks++;
                cout << "Block " << amount_blocks << " mit " << values.size() << " Messwerten fertig, Beginne Verarbeitung in XML mit: " << endl;
                cout << "#######################################################################" << endl;
                cout << "Anwendung: " << application_name << endl;
                cout << "Auftragsnummer: " << orno_number << endl;
                cout << "Kommentar: " << comment << endl;
                cout << "Stunde: " << block_hour << endl;
                cout << "Minute: " << block_minute << endl;
                cout << "Tag: " << block_day << endl;
                cout << "Monat: " << block_month << endl;
                cout << "Jahr: " << block_year << endl;
                cout << "Max: " << border_max << endl;
                cout << "Einheit: " << unit << endl;
                cout << "Min: " << border_min << endl;

                //Application-Element existiert nur einmal
                if (amount_blocks == 1)
                {
                    application->SetAttribute("name", application_name.data());
                    application->SetAttribute("max", border_max.data());
                    application->SetAttribute("min", border_min.data());
                    application->SetAttribute("unit", unit.data());
                    doc.LinkEndChild( decl );
                    doc.LinkEndChild( application );
                }
                TiXmlElement * block = new TiXmlElement( "block" );
                block->SetAttribute("ordernumber",orno_number.data());
                block->SetAttribute("day",block_day.data());
                block->SetAttribute("month",block_month.data());
                block->SetAttribute("year",block_year.data());
                block->SetAttribute("hour",block_hour.data());
                block->SetAttribute("minute",block_minute.data());
                block->SetAttribute("comment",comment.data());
                block->SetAttribute("amount", values.size());

                application->LinkEndChild(block);


                for (list<string>::const_iterator iterator = values.begin(), end = values.end(); iterator != end; ++iterator)
                {
                    //*iterator
                    string value = *iterator;
                    TiXmlElement * value_element = new TiXmlElement( "value" );
                    value_element->LinkEndChild( new TiXmlText(value.c_str()));
                    block->LinkEndChild(value_element);
                }
                values.clear();
            }

            if(regex_search(current_line, result, measure_value))
            {
                values.push_back(result.str("value"));
            }

            if(regex_search(current_line, result, block_comment))
            {
                comment = result.str("comment");
                orno_number = result.str("orno");
            }

            if (regex_search(current_line, result, application_regex))
            {
                application_name = result.str("app");
            }

            if (regex_search(current_line, result, date_regex))
            {
                block_day = result.str("day");
                block_month = result.str("month");
                block_year = result.str("year");
            }

            if (regex_search(current_line, result, time_regex))
            {
                block_hour = result.str("hour");
                block_minute = result.str("minute");
            }

            if (regex_search(current_line, result, max_value_regex))
            {
                border_max = result.str("max");
                unit = result.str("unit");
            }

            if (regex_search(current_line, result, min_value_regex))
            {
                border_min = result.str("min");
            }
        }
        cout << "Anzahl Bloecke: " << amount_blocks << endl;
        fs::path xml_file_name = "messdaten.xml";
        fs::path full_xml_path = timestamp_dir / xml_file_name;
        doc.SaveFile(full_xml_path.string().c_str());

        return 1;
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

