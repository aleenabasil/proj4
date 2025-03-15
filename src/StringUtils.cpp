#include "StringUtils.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

namespace StringUtils{

//Returns a substring from start to end indices (slicing)
std::string Slice(const std::string &str, ssize_t start, ssize_t end) noexcept{
   if(end == 0) {
    end = str.size();
    }
   if(start < 0) {
    start += str.size();
    }
   if(end < 0) {
    end += str.size();
   }
   return str.substr(start, end - start);
}
//Capitalize first letter and make the rest lowercase
std::string Capitalize(const std::string &str) noexcept{
    //if string is empty return
    if(str.empty()){
         return "";
    }
    std::string cap = str;
    //cap the first letter
    cap[0] = toupper(cap[0]);
    //make everything else lowercase
    for(size_t i = 1; i < cap.size(); i++){
        cap[i] = tolower(cap[i]);
    }
    return cap;

}

//Makes everything to caps
std::string Upper(const std::string &str) noexcept{
    //if string is empty return
    if(str.empty()) {
        return ""; 
    }
    std::string up = str;
    //convert all uppercase
    for(size_t i = 0; i < up.size(); i++){
        up[i] = toupper(up[i]);
    }
    return up;
}

//make everything to lowercase
std::string Lower(const std::string &str) noexcept{
    //if string is empty return
    if(str.empty()) {
        return "";
    }
    std::string low = str;
    //convert all lowercase
    for(size_t i = 0; i < low.size(); i++){
        low[i] = tolower(low[i]);
    }
    return low;
}

//removes the whitespace before the string
std::string LStrip(const std::string &str) noexcept{
    
    size_t start = 0;
    //skip spaces at start
    while(start < str.size() && isspace(str[start])) {
        start++; }
    //if string only spaces return
    if(start == str.size()) {
        return "";}
    return str.substr(start);

}

//removes the whitespace after the string
std::string RStrip(const std::string &str) noexcept{
    
    size_t end = str.size();
    //skip spaces at end
    while(end > 0 && isspace(str[end-1])) {
         end--; }
    //if string only spaces return
    if(end == 0) {
        return "";}
    return str.substr(0, end);
}

//removes whitespace on both sides
std::string Strip(const std::string &str) noexcept{
    // strip spaces at both start and end
    return LStrip(RStrip(str));
}

//makes the string center within the given width
std::string Center(const std::string &str, int width, char fill) noexcept{
    
    int num = width - str.size();
    if(num <= 0) {
        return str;}
    int left = num / 2;
    int right = num - left;
    //string on center
    return std::string(left, fill) + str + std::string(right, fill);
}

//aligns the string to the left
std::string LJust(const std::string &str, int width, char fill) noexcept{
    int num = width - str.size();
    if(num <= 0) {
        return str;}
    //string on left
    return str + std::string(num, fill);
}

//aligns the string to the right
std::string RJust(const std::string &str, int width, char fill) noexcept{
    int num = width - str.size();
    if(num <= 0) {
        return str;}
    //fill charaters on right
    return std::string(num, fill) + str;
}

//replace where all specific word with another word
std::string Replace(const std::string &str, const std::string &old, const std::string &rep) noexcept{
    std::string rp = str;
    size_t pos = 0;
    //find where old occurs
    while((pos = rp.find(old, pos)) != std::string::npos){
        //replace old with rep
        rp.replace(pos, old.length(), rep);
        //move past the rep length that was added
        pos += rep.length();
    }
    return rp;
}

//splits the string
std::vector< std::string > Split(const std::string &str, const std::string &splt) noexcept{
    std::vector<std::string> st;
    if(str.empty()) {
        return st;}
    size_t start = 0;
    size_t end;
    if(splt.empty()){
        while(start < str.size()){
            while(start < str.size() && std::isspace(str[start])) {
                start++;}
            if(start >= str.size()) {
                break;}
            end = start;
            //find the end of the word
            while(end < str.size() && !std::isspace(str[end])) {
                end++;}
            //remove then add to the new vector
            st.push_back(str.substr(start, end - start));
            start = end;
        }
    }
    else{
        while((end = str.find(splt, start)) != std::string::npos){
            st.push_back(str.substr(start, end - start));
            start = end + splt.length();
        }
        //add remaining substring
        st.push_back(str.substr(start));
    }
    return st;
}

//jois the elements into one string
std::string Join(const std::string &str, const std::vector< std::string > &vect) noexcept{
    std::string jn;
    if(vect.empty()) {
        return "";}
    for (size_t i = 0; i < vect.size(); i++){
        //append each element in the vector
        jn += vect[i];
        if(i < vect.size() - 1) jn += str;
    }
    return jn;
}
//replace tab characters with spaces
std::string ExpandTabs(const std::string &str, int tabsize) noexcept{
    std::string et;
    if(tabsize == 0){
        for(char c : str){
            if(c != '\t') {
                et += c;}
        }
        return et;
    }
    int count = 0;
    for(char c : str){
        //if tab found replace it with spaces
        if(c == '\t'){
            int spaces = tabsize - (count % tabsize);
            et.append(spaces, ' ');
            count += spaces;
        }
        else{
            //add the character to the string
            et += c;
            count ++;
        }
    }
    return et;
}

//computes the edit distance (levenshtein distance) between two strings
int EditDistance(const std::string &left, const std::string &right, bool ignorecase) noexcept{
    std::string l = left;
    std::string r = right;
    if(ignorecase){
        l = Lower(left);
        r = Lower(right);
    }

    std::vector<std::vector<int>> dp(l.size() + 1, std::vector<int>(r.size() + 1));

    for (size_t i = 0; i <= l.size(); i++) dp[i][0] = i;
    for (size_t j = 0; j <=r.size(); j++) dp[0][j] = j;

    //dynamic programming table
    for(size_t i = 1; i <= l.size(); i++){
       for(size_t j = 1; j <= r.size(); j++){
            int del = dp[i - 1][j] + 1;
            int ins = dp[i][j -1] + 1;

            char c1 = l[i -1];
            char c2 = r[j -1];
            int c = (c1 == c2) ? 0 : 1;
            int sub = dp[i - 1][j - 1] + c;

            dp[i][j] = std::min({del, ins, sub});
        }
    }
    return dp[l.size()][r.size()];

}

};
