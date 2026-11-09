#include "FileIntegrityTask.h"
#include "FileIntegrityExceptions.h"
#include <cassert>
#include <fstream>
#include <filesystem>
#include <iostream>
using namespace integrity;
int main(){
 auto f=std::filesystem::temp_directory_path()/"integrity_local_smoke.txt";
 {std::ofstream o(f);o<<"hello";}
 int passed=0;
 auto test=[&](bool x){if(!x) std::abort(); ++passed;};
 test(checksumFile(f,Algorithm::FNV1A64)=="a430d84680aabd0b");
 test(checksumFile(f,Algorithm::CRC32)=="3610a686");
 FileIntegrityTask t;test(!t.hasValue());
 try{t.info();test(false);}catch(const std::logic_error&){++passed;}
 t.analyze(f,Algorithm::CRC32);test(t.hasValue());test(t.info().size==5);
 test(t.verify("3610A686"));test(!t.verify("00000000"));
 try{t.verify("BAD");test(false);}catch(const ChecksumFormatException&){++passed;}
 try{t.verify("zzzzzzzz");test(false);}catch(const ChecksumFormatException&){++passed;}
 auto before=t.info();
 try{t.analyze(f.string()+".missing",Algorithm::CRC32);test(false);}catch(const FileAccessException&){++passed;}
 test(t.info()==before);
 try{t.analyze(f,static_cast<Algorithm>(99));test(false);}catch(const std::invalid_argument&){++passed;}
 test(t.info()==before);
 try{checksumFile(f.parent_path(),Algorithm::CRC32);test(false);}catch(const FileAccessException&){++passed;}
 {std::ofstream o(f,std::ios::trunc);}
 t.analyze(f,Algorithm::FNV1A64);test(t.info().size==0);test(t.info().checksum=="cbf29ce484222325");
 std::filesystem::remove(f);
 std::cout << "LOCAL SMOKE CHECKS PASSED: " << passed << "/" << passed << "\n";
}