#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <deque>
#include <algorithm>
#if defined ( __cplusplus ) && __cplusplus > 199711L /* C++ 11 */
#include <unordered_map>
#include <unordered_set>
#include <memory>
#endif /* C++ 11 */
using namespace std;
#endif /* __PROGTEST__ */

class CDate
{
  public:
    CDate(int year, int month, int day){
      d_year = year;
      d_month = month;
      d_day = day;
    }
    bool operator < (const CDate & b) const{
      if (d_year > b.d_year)
        return false;
      else if (d_year < b.d_year)
        return true;
      else{
        if (d_month > b.d_month)
          return false;
        else if (d_month < b.d_month)
          return true;
        else{
          if (d_day < b.d_day)
            return true;
          else
            return false;
        }
      }
    }
    bool operator >(const CDate & b) const{
      if (d_year < b.d_year)
        return false;
      else if (d_month > b.d_month)
        return true;
      else{
        if (d_month < b.d_month)
          return false;
        else if (d_month > b.d_month)
          return true;
        else{
          if (d_day > b.d_day)
            return true;
          else
            return false;
        }
      }
    }
    bool operator==(const CDate & b) const{
      if (d_year == b.d_year && d_month == b.d_month && d_day == b.d_day)
        return true;
      return false;
    }
    friend ostream & operator << (ostream & os, const CDate & b);    
  private:
    int d_year;
    int d_month;
    int d_day;
};

ostream & operator << (ostream & os, const CDate & b){
  return os << b.d_day << "." << b.d_month << "." << b.d_year;
}

struct CItem{
  CItem(CDate date, int count){
    inner.insert(pair<CDate, int>(date, count));
  }
  void Add(CDate date, int count){
    map<CDate, int>::iterator it = inner.find(date);
    if (it == inner.end()){
      inner.insert(pair<CDate, int>(date, count));   
    }else{
      it->second += count;
    }
  }
  map<CDate, int> inner;
};

bool listCmp(pair<string, int> & a, int b){
  if (a.second > b)
    return true;
  return false;
}

bool oneFaultCompare (string a, string b){
  if (a.size() != b.size())
    return false;
  int check = 0;
  for (uint i = 0; i < a.size(); i++){
    if (a[i] != b[i])
      check++;
    if(check>1)
      break;
  }
  if (check == 1){
    return true;
  }else{
    return false;
  }
}

bool listCompare ( pair <string, int> & prvni, pair <string, int> & druhy){
  return prvni.second > druhy.second;
}

class CSupermarket
{
  public:
    // default constructor
    CSupermarket & Store ( string name, CDate expiryDate, int count ){
      map<string, CItem>::iterator it = storage.find(name);
      if (it == storage.end()){
        CItem nowy = CItem(expiryDate, count);
        storage.insert(pair<string, CItem>(name, nowy));
      }else{
        it->second.Add(expiryDate, count);
      }
      return *this;
    }

    CSupermarket Sell ( list <pair <string, int>> & shoppingList ){
      list <pair <string, int>>::iterator here, there;
      list <pair <string, int>> realList;
      map<string, CItem>::iterator it;
      map<CDate, int>::iterator that;
      for (here = shoppingList.begin(); here != shoppingList.end(); here++){
        it = storage.find(here->first);
        if (it != storage.end()){
          realList.push_back(pair<string, int>(here->first, here->second));
          continue;
        }
        int check = 0;
        for (it = storage.begin(); it != storage.end(); it++){
          if (oneFaultCompare(here->first, it->first))
            check++;
          if (check > 1)
            break;
        }
        if (check != 1)
          continue;
        realList.push_back(pair<string, int>(here->first, here->second));
      }

      string similar;
      for (here = realList.begin(); here != realList.end(); here++){
        it = storage.find(here->first);
        if (it != storage.end()){
          similar = it->first;
        }else{
          int check = 0;
          for (it = storage.begin(); it != storage.end(); it++){
            if (oneFaultCompare(here->first, it->first)){
              similar = it->first;
              check++;
            }
            if (check > 1)
              break;
          }
          if (check != 1)
            continue;
          it = storage.find(similar);
        }
        that = it->second.inner.begin();
        int toSell = here->second;
        int deleted = 0;
        while(toSell > 0 && that != it->second.inner.end()){
          if (that->second > toSell){
            that->second -= toSell;
            deleted += toSell;
            toSell = 0;
          }else{
            toSell -= that->second;
            deleted += that->second;
            it->second.inner.erase(that++);
          }
        }
        if (that == it->second.inner.end())
          storage.erase(it);
        there = find(shoppingList.begin(), shoppingList.end(), pair<string, int>(here->first, here->second));
        if (there->second == deleted){
          shoppingList.erase(there);
        }else{
          there->second -= deleted;
        }
      }

/*
      for (here = shoppingList.begin(); here != shoppingList.end(); here++)
        cout << here->first << " " << here->second << endl; 
*/
      return *this;
    }

    list<pair <string, int>> Expired ( CDate date ){
      list<pair <string, int>> seznam;
      map<string, CItem>::iterator it;
      map<CDate, int>::iterator that;
      list<pair <string, int>>::iterator here;
      for (it = storage.begin(); it != storage.end(); it++){
        int sum = 0;
        that = it->second.inner.begin();
        while(that->first < date && that != it->second.inner.end()){
          sum += that->second;
          that++;
        }
        if (sum > 0){
          seznam.push_back(pair<string, int>(it->first, sum));
          seznam.sort(listCompare);
        }
      }
/*      
      for (here = seznam.begin(); here != seznam.end(); here++)
        cout << here->first << " " << here->second << endl;
*/
      return seznam;
    }

    void Show(){
      map<string, CItem>::iterator it;
      for (it = storage.begin(); it != storage.end(); it++){
        map<CDate, int>::iterator that;
        cout << it->first << endl;
        for (that = it->second.inner.begin(); that != it->second.inner.end(); that++)
          cout << " " << that->first << " " << that->second << endl;
        cout << endl;
      }
    }

  private:
    map<string, CItem> storage;
};

#ifndef __PROGTEST__
int main ( void )
{
  CSupermarket s;
  s . Store ( "bread", CDate ( 2016, 4, 30 ), 100 ) .
      Store ( "butter", CDate ( 2016, 5, 10 ), 10 ) .
      Store ( "beer", CDate ( 2016, 8, 10 ), 50 ) .
      Store ( "bread", CDate ( 2016, 4, 25 ), 100 ) .
      Store ( "okey", CDate ( 2016, 7, 18 ), 5 );

  list<pair<string,int> > l0 = s . Expired ( CDate ( 2018, 4, 30 ) );
  assert ( l0 . size () == 4 );
  assert ( ( l0 == list<pair<string,int> > { { "bread", 200 }, { "beer", 50 }, { "butter", 10 }, { "okey", 5 } } ) );

  list<pair<string,int> > l1 { { "bread", 2 }, { "Coke", 5 }, { "butter", 20 } };
  s . Sell ( l1 );
  assert ( l1 . size () == 2 );
  assert ( ( l1 == list<pair<string,int> > { { "Coke", 5 }, { "butter", 10 } } ) );


  list<pair<string,int> > l2 = s . Expired ( CDate ( 2016, 4, 30 ) );
  assert ( l2 . size () == 1 );
  assert ( ( l2 == list<pair<string,int> > { { "bread", 98 } } ) );
  list<pair<string,int> > l3 = s . Expired ( CDate ( 2016, 5, 20 ) );
  assert ( l3 . size () == 1 );
  assert ( ( l3 == list<pair<string,int> > { { "bread", 198 } } ) );

  list<pair<string,int> > l4 { { "bread", 105 } };
  s . Sell ( l4 );
  assert ( l4 . size () == 0 );
  assert ( ( l4 == list<pair<string,int> > {  } ) );

  list<pair<string,int> > l5 = s . Expired ( CDate ( 2017, 1, 1 ) );
  assert ( l5 . size () == 3 );
  assert ( ( l5 == list<pair<string,int> > { { "bread", 93 }, { "beer", 50 }, { "okey", 5 } } ) );



  s . Store ( "Coke", CDate ( 2016, 12, 31 ), 10 );

  list<pair<string,int> > l6 { { "Cake", 1 }, { "Coke", 1 }, { "cake", 1 }, { "coke", 1 }, { "cuke", 1 }, { "Cokes", 1 } };
  s . Sell ( l6 );
  assert ( l6 . size () == 3 );
  assert ( ( l6 == list<pair<string,int> > { { "cake", 1 }, { "cuke", 1 }, { "Cokes", 1 } } ) );

  list<pair<string,int> > l7 = s . Expired ( CDate ( 2017, 1, 1 ) );
  assert ( l7 . size () == 4 );
  assert ( ( l7 == list<pair<string,int> > { { "bread", 93 }, { "beer", 50 }, { "Coke", 7 }, { "okey", 5 } } ) );

  s . Store ( "cake", CDate ( 2016, 11, 1 ), 5 );

  list<pair<string,int> > l8 { { "Cake", 1 }, { "Coke", 1 }, { "cake", 1 }, { "coke", 1 }, { "cuke", 1 } };
  s . Sell ( l8 );
  assert ( l8 . size () == 2 );
  assert ( ( l8 == list<pair<string,int> > { { "Cake", 1 }, { "coke", 1 } } ) );

  list<pair<string,int> > l9 = s . Expired ( CDate ( 2017, 1, 1 ) );
  assert ( l9 . size () == 5 );
  assert ( ( l9 == list<pair<string,int> > { { "bread", 93 }, { "beer", 50 }, { "Coke", 6 }, { "okey", 5 }, { "cake", 3 } } ) );

  list<pair<string,int> > l10 { { "cake", 15 }, { "Cake", 2 } };
  s . Sell ( l10 );
  assert ( l10 . size () == 2 );
  assert ( ( l10 == list<pair<string,int> > { { "cake", 12 }, { "Cake", 2 } } ) );

  list<pair<string,int> > l11 = s . Expired ( CDate ( 2017, 1, 1 ) );
  assert ( l11 . size () == 4 );
  assert ( ( l11 == list<pair<string,int> > { { "bread", 93 }, { "beer", 50 }, { "Coke", 6 }, { "okey", 5 } } ) );

  list<pair<string,int> > l12 { { "Cake", 4 } };
  s . Sell ( l12 );
  assert ( l12 . size () == 0 );
  assert ( ( l12 == list<pair<string,int> > {  } ) );

  list<pair<string,int> > l13 = s . Expired ( CDate ( 2017, 1, 1 ) );
  assert ( l13 . size () == 4 );
  assert ( ( l13 == list<pair<string,int> > { { "bread", 93 }, { "beer", 50 }, { "okey", 5 }, { "Coke", 2 } } ) );

  list<pair<string,int> > l14 { { "Beer", 20 }, { "Coke", 1 }, { "bear", 25 }, { "beer", 10 } };
  s . Sell ( l14 );
  assert ( l14 . size () == 1 );
  assert ( ( l14 == list<pair<string,int> > { { "beer", 5 } } ) );


  return 0;
}
#endif /* __PROGTEST__ */
