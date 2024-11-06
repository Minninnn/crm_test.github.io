#include <iostream>
#include <fstream>
#include <pqxx/pqxx>
#include <restbed>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

string load_file(const string& filepath)
{
    ifstream file(filepath);
    if (!file.is_open()) {
        throw runtime_error("Unable to open file");
    }
    return string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

void get_index_handler(const shared_ptr<restbed::Session> session)
{
    try
    {
        // Загрузите HTML из файла
        const string response = load_file("D:/Projects/DBProject/test/Static/index.html"); 

        session->close(restbed::OK, response, { {"Content-Length", to_string(response.size())}, {"Content-Type", "text/html"} });
    }
    catch (const exception& e) {
        cerr << "Error loading HTML file: " << e.what() << endl;
        session->close(restbed::INTERNAL_SERVER_ERROR, "Error loading HTML file", { {"Content-Length", "24"} });
    }
}

void get_success_handler(const shared_ptr<restbed::Session> session) 
{
    try {
        const string response = load_file("D:/Projects/DBProject/test/Static/PageMain.html");
        session->close(restbed::OK, response, { {"Content-Length", to_string(response.size())}, {"Content-Type", "text/html"} });
    }
    catch (const exception& e) {
        cerr << "Error loading HTML file: " << e.what() << endl;
        session->close(restbed::INTERNAL_SERVER_ERROR, "Error loading HTML file", { {"Content-Length", "24"} });
    }
}


void css_handler(const shared_ptr<restbed::Session> session)
{
    try {
        const string response = load_file("D:/Projects/DBProject/test/Static/custom.css");
        session->close(restbed::OK, response, { {"Content-Length", to_string(response.size())}, {"Content-Type", "text/css"} });
    }
    catch (const exception& e) {
        cerr << "Error loading CSS file: " << e.what() << endl;
        session->close(restbed::INTERNAL_SERVER_ERROR, "Error loading CSS file", { {"Content-Length", "23"} });
    }
}

void post_method_handler(const shared_ptr<restbed::Session> session) {
    const auto request = session->get_request();
    int content_length = request->get_header("Content-Length", 0);

    session->fetch(content_length, [](const shared_ptr<restbed::Session> session, const restbed::Bytes& body) {
        try {
            auto request_body = string(reinterpret_cast<const char*>(body.data()), body.size());
            auto data = json::parse(request_body);

            if (!data.contains("name") || !data.contains("surname") || !data.contains("phone_number") || !data.contains("email")) {
                session->close(restbed::BAD_REQUEST, "Missing name, surname, phone_number, or email", { {"Content-Length", "50"} });
                return;
            }

            string name = data["name"];
            string surname = data["surname"];
            string phone_number = data["phone_number"];
            string email = data["email"];

            if (name.empty() || surname.empty() || phone_number.empty() || email.empty()) {
                session->close(restbed::BAD_REQUEST, "Empty fields are not allowed", { {"Content-Length", "28"} });
                return;
            }

            pqxx::connection c("user=postgres port=5432 dbname=test password=1060 host=localhost");
            pqxx::work txn(c);

            txn.exec("INSERT INTO clients_info (name, surname, phone_number, email) VALUES (" + txn.quote(name) + ", " + txn.quote(surname) + ", " + txn.quote(phone_number) + ", " + txn.quote(email) + ")");
            txn.commit();

            session->close(restbed::SEE_OTHER, "", { {"Location", "/PageMain"} });
        }
        catch (const json::exception& e) {
            cerr << "JSON parsing error: " << e.what() << endl;
            session->close(restbed::BAD_REQUEST, "Invalid JSON format", { {"Content-Length", "20"} });
        }
        catch (const pqxx::sql_error& e) {
            cerr << "SQL error: " << e.what() << endl;
            session->close(restbed::INTERNAL_SERVER_ERROR, "Database error", { {"Content-Length", "14"} });
        }
        catch (const exception& e) {
            cerr << "Error: " << e.what() << endl;
            session->close(restbed::INTERNAL_SERVER_ERROR, "Server error", { {"Content-Length", "12"} });
        }
        });   
}

int main() {

    shared_ptr<restbed::Resource> resource = make_shared<restbed::Resource>();

     resource->set_path("/clients");

     resource->set_method_handler("GET", get_index_handler);

     resource->set_method_handler("POST", post_method_handler);

     shared_ptr<restbed::Resource> success_resource = make_shared<restbed::Resource>(); 
     success_resource->set_path("/PageMain");
     success_resource->set_method_handler("GET", get_success_handler);
        
     shared_ptr<restbed::Resource> css_resource = make_shared<restbed::Resource>();
     css_resource->set_path("/custom.css");
     css_resource->set_method_handler("GET", css_handler);

     shared_ptr<restbed::Settings> settings = make_shared<restbed::Settings>();

     settings->set_port(8080);

     restbed::Service service;
     service.publish(resource);
     service.publish(success_resource);
     service.publish(css_resource);
     service.start(settings);

     return 0;
}       