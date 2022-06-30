//#include <vector>
//#include <glm/glm.hpp>
//#include <string>
//#include <iostream>
//#include <fstream>
//#include <sstream>
//#include <regex>
//
//struct chess_move {
//    unsigned int index;
//    std::string type;
//    std::string p1;
//    std::string p2;
//};
//
//
//void loadGame(const std::string path) {
//
//    std::ifstream file(path);
//    std::string line;
//
//    std::regex pattern("^(\\d*) (\\S*) (\\S*)(:? (\\S*))?");
//    std::vector<chess_move> moves;
//
//    while (std::getline(file, line)) {
//        std::smatch result;
//        if (std::regex_search(line, result, pattern))
//            std::cout << "Linia " << line << " : [" << result[2] << ']\n';
//        else
//            std::cout << "error\n";
//    }
//}