#pragma once

#include <string>
#include <math.h>

using namespace std;

namespace egame {

   void merge_hash(capi_checksum256 &s1, capi_checksum256 s2) {
      for (int i = 0; i < 32; ++i) {
         s1.hash[i] ^= s2.hash[31-i];
      }
   }

   bool is_zero(const capi_checksum256& a) {
      const uint64_t *p64 = reinterpret_cast<const uint64_t*>(&a);
      return p64[0] == 0 && p64[1] == 0 && p64[2] == 0 && p64[3] == 0;
   }

   void set_zero(const capi_checksum256& a) {
      memset( (void *)&a, 0, sizeof(capi_checksum256) );
   }

   bool is_equal(const capi_checksum256& a, const capi_checksum256& b) {
      return memcmp((void *)&a, (const void *)&b, sizeof(capi_checksum256)) == 0;
   }

   vector<string> split(const string& str, const string& delim) {  
      vector<string> res;  
      if("" == str) return res;  
      char * strs = new char[str.length() + 1] ;  
      strcpy(strs, str.c_str());   

      char * d = new char[delim.length() + 1];  
      strcpy(d, delim.c_str());  

      char *p = strtok(strs, d);  
      while(p) {  
         string s = p; 
         res.push_back(s);
         p = strtok(NULL, d);  
      }   
      return res;  
   }

   uint8_t from_hex(char c) {
      if (c >= '0' && c <= '9') return c - '0';
      if (c >= 'a' && c <= 'f') return c - 'a' + 10;
      if (c >= 'A' && c <= 'F') return c - 'A' + 10;
      eosio_assert(false, "Invalid hex character");
      return 0;
   }

   size_t from_hex(const std::string& hex_str, char* out_data, size_t out_data_len) {
      auto i = hex_str.begin();
      uint8_t* out_pos = (uint8_t*)out_data;
      uint8_t* out_end = out_pos + out_data_len;
      while (i != hex_str.end() && out_end != out_pos) {
         *out_pos = from_hex((char)(*i)) << 4;
         ++i;
         if (i != hex_str.end()) {
               *out_pos |= from_hex((char)(*i));
               ++i;
         }
         ++out_pos;
      }
      return out_pos - (uint8_t*)out_data;
   }

   capi_checksum256 hex_to_sha256(const std::string& hex_str) {
      eosio_assert(hex_str.length() == 64, "invalid sha256");
      capi_checksum256 checksum;
      from_hex(hex_str, (char*)checksum.hash, sizeof(checksum.hash));
      return checksum;
   }

   uint64_t get_send_id( uint128_t sender_id )
   {
      return uint64_t( sender_id >> 64 );
   }

   template<typename T>
   uint32_t get_size( T& table ) {
      uint32_t count = 0;
      auto itr = table.begin();
      while( itr != table.end() ) {
         count++;
         itr++;
      }
      return count;
   }

   template<typename T>
   void clear_table( T& table, uint32_t max = 500 ) {
      auto itr = table.begin();
      uint32_t cur = 0;
      while( itr != table.end() && cur < max ) {
         itr = table.erase( itr );
         cur++;
      }
   }

}