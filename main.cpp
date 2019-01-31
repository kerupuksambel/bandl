#include <iostream>
#include <string>
#include <math.h>
#include <curl/curl.h>
#include <yaml.h>
#include <fstream>


#define LOG(X) std::cout << X << std::endl;
#define MEMBER_TOTAL 5
#define BAND_TOTAL 5
#define CARD_TOTAL 5

enum Card { NORMAL=1, TRAINED };

struct Parameter
{
	std::string card_type;
	unsigned int card_value;
	unsigned int isTrained;
	std::string outputDir;
	bool verbose;

	Parameter(): 
		card_type("-a"), 
		card_value(0), 
		isTrained(Card::NORMAL | Card::TRAINED),
		outputDir("./"),
		verbose(false) {}

};

std::string generateId(int& value, int digit)
{
	std::string container;
	int x = floor(log10(value)) + 1;
	for (int i = 0; i < (digit - x); i++)
	{
		container.append("0");	
	}
	return container.append(std::to_string(value));
}

std::string generateUrl(int& member_id, int& card_id, std::string type)
{
	std::string url = "https://res.bandori.ga/assets-jp/characters/resourceset/res:i::e:_rip/card_:t:.png";
	url.replace(59, 3, generateId(member_id,3));
	url.replace(62, 3, generateId(card_id,3));
	url.replace(75, 3, type);
	return url;
}

std::size_t write_img(char* buffer, size_t size, size_t nmemb, void* userdata)
{
	std::ofstream* out = static_cast<std::ofstream*> (userdata);
	std::size_t total_bytes = size * nmemb;
	out->write(buffer, total_bytes);
	return total_bytes;
}

CURLcode downloadImage(CURL* curl, std::ofstream& file, std::string url)
{
	file.open("temp_data", std::ios::binary);

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_img);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	
	return curl_easy_perform(curl);
}

int main(int argc, char const *argv[])
{

	Parameter param;
	curl_global_init(CURL_GLOBAL_DEFAULT);
	CURL* curl = curl_easy_init();
	std::ofstream out;
	CURLcode res;
	std::string filename ;
	std::string url ;
	long response_code;

	YAML::Node data = YAML::LoadFile("data.yml");
	//pass by reference karena value bukan literal alias ngambil variable lain
	//LOG(url.replace(59, 3, generateId(5, 3)));
	//LOG(url.replace(62, 3, generateId(19, 3)));
	//LOG(url.replace(75, 3, "after_training"));
	if (argc <= 1)
	{
		std::cout << "Bandori Downloader v0.1b" << std::endl;
		std::cout << "There's no given argument, exiting.." << std::endl;
	}
	if (argc > 1)
	{
		for (argc--, argv++; *argv; argc--, argv++)
		{
			if (strncmp(*argv, "-a", 2) == 0) {
				param.card_type = "all";
				param.card_value = 0;
				
			} else if (strncmp(*argv, "-b=", 3) == 0) {
				param.card_type = "band";									
				for (int i=0; i < MEMBER_TOTAL; i++)
				{
					if (data["band"][i].as<std::string>().compare(((*argv) + 3)) == 0) {
						param.card_value = i + 1;
						break;
					}
				}
			} else if (strncmp(*argv, "-c=", 3) == 0) {
				param.card_type = "chara";
				for (YAML::const_iterator it=data["character"].begin(); it!=data["character"].end(); ++it)
				{
					if (it->first.as<std::string>().compare((*argv)+3) == 0) {
						param.card_value = (it->second["band"].as<int>() * BAND_TOTAL) + it->second["position"].as<int>();
						break;
					}
				}
			} else if (strncmp(*argv, "-i=", 3) == 0) {
				param.card_type = "id";
				param.card_value = 1;
			} else if (strncmp(*argv, "-t=", 3) == 0) {
				if ( ((*argv)+3)[0] == 121 )
					param.isTrained = Card::TRAINED;
				else if ( ((*argv)+3)[0] == 110 )
					param.isTrained = Card::NORMAL;
				else
					LOG("Anda salah input")
			}
		}
		
		int finish;
		int start;

		if (param.card_type.compare("band") == 0) {
			finish = param.card_value * BAND_TOTAL;
			start = (finish - MEMBER_TOTAL) + 1;
		} else {
			finish = param.card_value;
			start = param.card_value;
		}
		
		while (start <= finish) 
		{
			if (param.card_type.compare("id") != 0) {
				for (int i = 1; i <= CARD_TOTAL; i++) 
				{
					switch(param.isTrained) {
						case Card::NORMAL:
							url = generateUrl(start, i, "normal");
							filename = url.substr(56,9).append(url.substr(url.find("card_")));
							res = downloadImage(curl, out, url);
							out.close();
							if (res == CURLE_OK) {
								res = curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &response_code);
								if (!res && response_code == 200) {
									rename("temp_data", filename.c_str());
									std::clog << filename << " downloaded" << std::endl;
								} else {
									remove("temp_data");
									std::clog << filename << " not found" << std::endl;
								}
							} else {
								std::cerr << curl_easy_strerror(res) << std::endl;
							}
							break;
						case Card::TRAINED:
							url = generateUrl(start, i, "after_training");
							filename = url.substr(56,9).append(url.substr(url.find("card_")));
							res = downloadImage(curl, out, url);
							out.close();
							if (res == CURLE_OK) {
								res = curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &response_code);
								if (!res && response_code == 200) {
									rename("temp_data", filename.c_str());
									std::clog << filename << " downloaded" << std::endl;
								} else {
									remove("temp_data");
									std::clog << filename << " not found" << std::endl;
								}
							} else {
								std::cerr << curl_easy_strerror(res) << std::endl;
							}
							break;
						default:
							LOG("not yet implemented")
							break;
					}
				}
			}
			start++;
		}
	}

	curl_easy_cleanup(curl);
	curl_global_cleanup();
	return 0;
}						