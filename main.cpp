#include <algorithm>
#include <curlpp/Easy.hpp>
#include <curlpp/Exception.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/cURLpp.hpp>
#include <iostream>
#include <math.h>
#include <rapidjson/document.h>
#include <rapidjson/encodings.h>
#include <rapidjson/error/en.h>
#include <sstream>
#include <sys/stat.h>
#include <vector>

const char *m_Name = R"(


    _   ____  __           __        _    ____                    
   / | / / / / /__  ____  / /_____ _(_)  / __ \____ _      ______ 
  /  |/ / /_/ / _ \/ __ \/ __/ __ `/ /  / / / / __ \ | /| / / __ \
 / /|  / __  /  __/ / / / /_/ /_/ / /  / /_/ / /_/ / |/ |/ / / / /
/_/ |_/_/ /_/\___/_/ /_/\__/\__,_/_/  /_____/\____/|__/|__/_/ /_/ 
                                                                  


)";

//execute a given command and return output
std::string exec(std::string command) {
  char buffer[128];
  std::string result = "";
  FILE *pipe = popen(command.c_str(), "r");

  if (!pipe) {
    return "Error: Popen Failed";
  }

  while (!feof(pipe)) {
    if (fgets(buffer, 128, pipe) != NULL) {
      result += buffer;
    }
  }

  pclose(pipe);
  return result;
}

//a cute split function
std::vector<std::string>
split(std::string s, std::string delimiter) {
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  std::string token;
  std::vector<std::string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }

  res.push_back(s.substr(pos_start));
  return res;
}

//Get using curlPP
std::string curlGet(std::string url) {
  std::stringstream ss;

  try {
    curlpp::Cleanup cleaner;
    curlpp::Easy request;

    request.setOpt(new curlpp::options::Url(url));
    request.setOpt(new curlpp::options::WriteStream(&ss));
    request.perform();

  } catch (curlpp::LogicError &e) {
    std::cout << e.what() << std::endl;
  } catch (curlpp::RuntimeError &e) {
    std::cout << e.what() << std::endl;
  }
  return ss.str();
}

//rapidjson Parser for nhentai
rapidjson::Document parseJson(std::string strWJson) {
  auto r = split(split(strWJson, "JSON.parse(")[1], ");");
  std::string res = r[0];

  rapidjson::Document doc;
  doc.Parse(res.c_str());

  rapidjson::Document d;
  d.Parse(doc.GetString());

  return d;
}

//thinking back this may not be necessary
std::vector<std::string> getImages(std::string media_id, int total) {
  std::vector<std::string> m_Images;

  for (int i = 0; i < total; i++) {
    //std::cout << "https://i.nhentai.net/" << media_id << "/" << i + 1 << std::endl;
    m_Images.push_back("https://i.nhentai.net/galleries/" + media_id + "/" + std::to_string(i + 1) + ".jpg");
  }

  return m_Images;
}

//callback for curlPP writing
size_t WriteCallback(char *ptr, size_t size, size_t nmemb, void *f) {
  FILE *file = (FILE *)f;
  return fwrite(ptr, size, nmemb, file);
};

//this is unnecessary too but gets cookie
std::string getToken(std::string page) {
  auto res = split(split(page, "csrf_token: \"")[1], "\",");
  return res[0];
}

void downloadImg(std::string url, std::string filename, std::string cookie) {
  try {
    curlpp::Cleanup cleaner;
    curlpp::Easy request;

    std::list<std::string> header;
    header.push_back("Accept: image/webp,*/*");
    request.setOpt(new curlpp::options::HttpHeader(header));

    request.setOpt(new curlpp::options::Url(url));
    request.setOpt(curlpp::options::Cookie(cookie));
    // request.setOpt(new curlpp::options::Verbose(true));

    /// Set the writer callback to enable cURL to write result in a memory area
    curlpp::options::WriteFunctionCurlFunction
        myFunction(WriteCallback);
    FILE *file = stdout;

    if (filename.c_str() != NULL) {
      file = fopen(filename.c_str(), "wb");
      if (file == NULL) {
        fprintf(stderr, "%s/n", strerror(errno));
      }
    }

    curlpp::OptionTrait<void *, CURLOPT_WRITEDATA>
        myData(file);

    request.setOpt(myFunction);
    request.setOpt(myData);

    request.perform();

  } catch (curlpp::LogicError &e) {
    std::cout << e.what() << std::endl;
  } catch (curlpp::RuntimeError &e) {
    std::cout << e.what() << std::endl;
  }
}

bool IsPathExist(const std::string &s) {
  struct stat buffer;
  return (stat(s.c_str(), &buffer) == 0);
}

void createDir(std::string f_name) {
  if (!IsPathExist(f_name)) {
    mkdir(f_name.c_str(), 0777);
  }
}

int main(int argc, char *argv[]) {

  std::cout << m_Name << std::endl;

  if (argc < 2) {
    std::cout << "Usage : " << argv[0] << "\tNhentai Sauce Num" << std::endl
              << "Example : " << argv[0] << "\t177013" << std::endl;
    return 1;
  }
  std::string hNum = argv[1];
  auto res = curlGet("https://nhentai.net/g/" + hNum + "/");

  //get cookie
  std::string csrf = getToken(res);
  std::string cookie = "csrftoken=" + csrf;

  //parse json in html
  rapidjson::Document rapidDoc = parseJson(res);
  auto imgs = getImages(rapidDoc["media_id"].GetString(), rapidDoc["num_pages"].GetInt());

  //create a folder and dl imgs inside
  createDir("./_tmp");
  createDir("./_tmp/" + hNum);
  std::string path = "./_tmp/" + hNum + "/";
  std::cout << path;
  for (unsigned int i = 0; i < imgs.size(); i++) {
    downloadImg(imgs[i], path + std::to_string(i) + ".jpg", cookie);
  }

  //create cbz & delete tmp folder
  std::string cmd = rapidDoc["title"]["english"].GetString();
  exec("zip \"" + cmd + ".cbz\" " + path + "*");
  exec("rm -rf ./_tmp");
  return 0;
}

/*
 * Written in C++
 * IDE Used Juci++
 * Author: kr3ypt0n
 * Github: https://github.com/krypton-kry/
 */