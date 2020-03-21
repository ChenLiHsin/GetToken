/*
 * File:   main.cpp
 * Author: COM
 *
 * Created on 2018年4月6日, 下午 2:33
 */

// Token有三種種類
// Case1:IDENTIFIER (底線或英文字母開頭，之後跟著英文字母或底線或數字)
//       Ex:int, _abc, k9999, a, cc, kfn_0, a2b, return, ...
// Case2:數字 與 字串(數字可能會有小數點，字串(被2個"夾起來 不跨行))
//       Ex:1, 565, 5.22, 0.2, "hello 5.22", "9852_final&&",...
// Case3:特殊符號(以下所列出來的37種特殊符號)
//       +   -   *   /   >   <   =   <=   >=   ==   !=   ?   %   &   &&
//       ||   |   ^   "   '   .   ,   (   )   [   ]   {   }   <<   >>
//       !   :   ;   #   ->  ++ --

// *如何切token?
// 1. 跳過所有white-space及註解、直到碰到「非white-space」(或'END_OF_FILE'這個字)為止
//    ps: white-space 是 空白, Tab, 換行
//        註解 「//」到end line 或 夾在「/*」與「*/」之間
// 2. 從那個「非white-space」開始讀入、且盡可能的往後讀，直到碰到「white-space」 或 「前面所提到的特殊字元」 或 「註解」為止
// 3. 此時你已讀入一個token了，問題只是「它是哪個case」而已

// 最後要完成以下功能
// 1.總共有多少個 token
// 2.三種case各有多少 token
// 3.印出指定 token 的位置 (line number,column number)
// 4.印出所指定的那一 line 出現哪幾個 token (要排序)
// 5.結束

# include <stdio.h>
# include <string.h>
# include <iostream>

using namespace std ;

enum TokenType { IDENTIFIER = 34512, CONSTANT = 87232, SPECIAL = 29742 } ;

typedef char *CharPtr ;

struct Column {
    int column ; // 此token(的字首)在本line有出現於此column
    Column *next ; // 下一個此token(在本line)有出現的column
} ; // Column

typedef Column *ColumnPtr ;

struct Line { // struct Line 記錄了4種資訊
    int line ; // 此token放在哪一line
    ColumnPtr firstAppearAt ; // 指到此token第一次出現在此line的哪一column
    ColumnPtr lastAppearAt ; // 指到此token最後出現在此line的哪一column
    Line *next ; // 同一token下次出現在哪一line
} ;

typedef Line *LinePtr ;

struct Token { // Struct token 記錄了4種資訊
    CharPtr tokenStr ; // tokenStr 放你切下來的token
    TokenType type ; // type 記錄此token是屬於哪一個case
    LinePtr firstAppearOn ; // 此token第一次出現在哪一line
    LinePtr lastAppearOn ; // 此token最後出現在哪一line
    Token *next ; // 指向下一個token
} ; // struct Token

typedef Token *TokenPtr ;

TokenPtr gFront = NULL, gRear = NULL ; // 分別指向Token串列的頭跟尾

typedef char Str100[ 100 ] ; // 此型別是打算用來宣告一個陣列變數、以供讀字串之用

int gLine = 1 ; // 「下一個要讀進來的字元」所在的line number

int gColumn = 1 ; // 「下一個要讀進來的字元」所在的column number

CharPtr GetToken( Str100 scanfStr ) { // GetToken()可能會有一些參數
    
    Str100 tokenStr = { 0 } ; // 初始化
    
    CharPtr returnTokenStr = NULL ;
    
    strcpy( tokenStr, scanfStr ) ; // 現在tokenStr裡面放的是我們讀到的token字串
    
    returnTokenStr = new char[ strlen( tokenStr ) + 1 ] ;
    strcpy( returnTokenStr, tokenStr ) ;
    
    return returnTokenStr ;
    
} // GetToken()

TokenPtr FindToken( CharPtr str, TokenPtr tokenPtr ) {
    // 此function只是在找token，沒有新增或刪除節點，故不用＆
    TokenPtr walk = NULL ;
    walk = tokenPtr ; // assert:第一次會指到gFront指到的第一個節點
    while ( walk != NULL && strcmp( walk -> tokenStr, str ) != 0 ) {
        walk = walk -> next ;
    }
    
    return walk ;
} // FindToken()

// 用來更新token的function

//-------------------------------------------------------------------是否要加＆-------------------------------------------------------------------------------------

void UpdateToken( CharPtr str, TokenPtr tokenPtr ) {
    tokenPtr = FindToken( str, tokenPtr ) ;
    ColumnPtr walkColumn = NULL ; // 用來追蹤是在哪一個column新增
    LinePtr walkLine = NULL ; // 用來追蹤在哪一個
    
    //每次更新都要重跑line
    
    // 若第二次出現的token和最後一次出現的在同一行就更新column
    if ( gLine == tokenPtr -> lastAppearOn -> line ) {
        tokenPtr -> firstAppearOn -> lastAppearAt -> column = gColumn ;
        
        walkColumn = tokenPtr -> firstAppearOn -> firstAppearAt ;
        while ( walkColumn != NULL ) {
            if ( walkColumn -> next == NULL ) {
                walkColumn -> next = new Column ;
                walkColumn -> next -> next = NULL ;
                walkColumn -> next -> column = gColumn ;
                tokenPtr -> firstAppearOn -> lastAppearAt = walkColumn -> next ;
            } // if()
            else {
                walkColumn = walkColumn -> next ;
            } // else()
            
        } // while()
    } // if()
    // 若第二次出現的token和第一次出現的在不同行就新增一個line
    else {
        // 找到firstAppearOn接到的最後一個NULL，用來新增這一個新line
        walkLine = tokenPtr -> firstAppearOn ;
        while ( walkLine != NULL ) {
            if ( walkLine -> next == NULL ) {
                // 新增完一個新的line後要令lastLine = 這個新的
                walkLine -> next = new Line ;
                walkLine -> next -> next = NULL ; // 先讓這個新的（目前此token的最後一個line節點指向NULL)
                walkLine -> next -> line = gLine ;
                walkLine -> next -> firstAppearAt -> column = gColumn ;
                walkLine -> next -> firstAppearAt -> next = NULL ;
                walkLine -> next -> lastAppearAt = walkLine -> next -> firstAppearAt ;
                tokenPtr -> lastAppearOn = walkLine -> next ;
            } // if()
            else {
                walkLine = walkLine -> next ;
            } // else()
        } // while()
    } // else()
    
} // UpdateToken()

// 用來新增資料的function
// 新增token還要有排序(按照ASCII code)功能
// 若此token尚未出現過就新增一個token並設定裡面的參數（尚未用到的指標要設成NULL）
void AddNewToken( CharPtr tokenName, TokenType type, TokenPtr &tokenPtr ) {
    // 加入新的token會有三種情況
    // 第一種是加在第一個（gFront = new Token)
    // 第二種是加在中間，walk -> tokenStr < str && walk -> next -> tokenStr > str
    // 第三種情況是加在最後，當walk = NULL時，walk = new Token
    
    TokenPtr walk = NULL ;
    TokenPtr copy = NULL ;
    walk = tokenPtr ;
    
    // 第一次需直接新增
    if ( gFront == NULL ) {
        gFront = new Token ;
        gFront -> tokenStr = tokenName ;
        gFront -> type = type ;
        gFront -> firstAppearOn -> line = gLine ;
        gFront -> firstAppearOn -> firstAppearAt -> column = gColumn ;
        gFront -> firstAppearOn -> firstAppearAt -> next = NULL ;
        gFront -> firstAppearOn -> lastAppearAt = tokenPtr -> firstAppearOn -> firstAppearAt ;
        gFront -> firstAppearOn -> next = NULL ;
        gFront -> lastAppearOn = tokenPtr -> firstAppearOn ;
        gFront -> next = NULL ;
        
        /*
        printf( "%s\n", tokenPtr -> tokenStr ) ;
        printf( "type = %d\n", tokenPtr -> type ) ;
        printf( "位置( %d,%d )\n", tokenPtr -> firstAppearOn -> line, tokenPtr -> firstAppearOn -> firstAppearAt -> column = gColumn  ) ;
        return ;
         */
        
    } // if()
    
    // 第二次開始進迴圈
    while ( walk != NULL ) {
        
      // 狀況一：加在最前面，後面有東西
      if ( tokenName < tokenPtr -> tokenStr ) {
          copy = tokenPtr ;
          tokenPtr = new Token ; // 此時新增了第一個節點（最小的token）
          tokenPtr -> tokenStr = tokenName ;
          tokenPtr -> type = type ;
          tokenPtr -> firstAppearOn -> line = gLine ;
          tokenPtr -> firstAppearOn -> firstAppearAt -> column = gColumn ;
          tokenPtr -> firstAppearOn -> firstAppearAt -> next = NULL ;
          tokenPtr -> firstAppearOn -> lastAppearAt = tokenPtr -> firstAppearOn -> firstAppearAt ; // lastColumn先初始化和firstColumn一樣
          tokenPtr -> firstAppearOn -> next = NULL ; // 第二種line出現會用到這個
          tokenPtr -> lastAppearOn = tokenPtr -> firstAppearOn ; // 這個token出現第二次的時候是放在這裡
          tokenPtr -> next = copy ;
          
      } // if()
    
      // 狀況二：加在中間
      else if ( walk -> tokenStr < tokenName && tokenName < walk -> next -> tokenStr ) {
          copy = walk -> next ;
          walk -> next = new Token ; // 插入新結點
          walk -> tokenStr = tokenName ;
          walk -> type = type ;
          walk -> firstAppearOn -> line = gLine ;
          walk -> firstAppearOn -> firstAppearAt -> column = gColumn ;
          walk -> firstAppearOn -> firstAppearAt -> next = NULL ;
          walk -> firstAppearOn -> lastAppearAt = walk -> firstAppearOn -> firstAppearAt ;
          walk -> firstAppearOn -> next = NULL ; // 若是第二個此token出現就需要讓這個指標指向last
          walk -> lastAppearOn = walk -> firstAppearOn ;
          walk -> next = copy ;
      } // else if()
        
      // 狀況三：加在最後
      if ( tokenName > walk -> tokenStr && walk -> next == NULL ) {
          walk = walk -> next ;
          walk = new Token ;
          walk -> tokenStr = tokenName ;
          walk -> type = type ;
          walk -> firstAppearOn -> line = gLine ;
          walk -> firstAppearOn -> firstAppearAt -> column = gColumn ;
          walk -> firstAppearOn -> firstAppearAt -> next = NULL ;
          walk -> firstAppearOn -> lastAppearAt = walk -> firstAppearOn -> firstAppearAt ;
          walk -> firstAppearOn -> next = NULL ; // 若是第二個此token出現就需要讓這個指標指向last
          walk -> lastAppearOn = walk -> firstAppearOn ;
          walk -> next = NULL ;
        } // if()
        
      walk = walk -> next ;
        
    } // while()
    
} // AddNewToken()

void DigitToFormal( CharPtr str ) {
    // 數字有多種情況不合理
    // 05 (零開頭的數字，把零去掉）
    // .54 （點開頭的數字，前面加個零）
    Str100 copyStr = "\0" ;
    int j = 0 ;
    
    if ( str[0] == '0' ) {
        for ( int i = 1 ; i < strlen( str ) ; i ++ ) {
            copyStr[j] = str[i] ;
            j ++ ;
        } // for()
        
        strcpy( str, copyStr ) ;
    } // if()
    else if ( str[0] == '.' ) {
        strcpy( copyStr, str ) ;
        for ( int i = 0 ; i < 100 ; i++ )
            str[i] = '\0' ;
        
        str[0] = '0' ;
        strcat( str, copyStr ) ;
    } // else if()
    
} // DigitToFormal()

bool TokenHasExist( CharPtr str, TokenPtr tokenPtr ) {
    TokenPtr walk = NULL ;
    walk = tokenPtr ;
    while ( walk != NULL ) {
        if ( strcmp( walk -> tokenStr, str ) == 0 )
            return true ;
        
        walk = walk -> next ;
    } // while()
    
    return false ;
} // TokenHasExist()

// 處理Case1
void ProcessIdentifier( char ch, TokenPtr &tokenPtr, bool &eof ) {
    Str100 scanStr = "\0" ;
    CharPtr tokenName = NULL ;
    int i = 0 ; // 用來計算scanStr中是第幾個index
    char nextChar = '\0' ;
    nextChar = cin.peek() ; // 先看第一個尚未讀進的字元是什麼，若也是a~z||A~Z||_則繼續讀
    scanStr[i] = ch ;
    i ++ ;
    
    while ( ( nextChar >= 'A' && nextChar <= 'Z' ) || ( nextChar >= 'a' && nextChar <= 'z' ) || nextChar == '_' ) {
        scanf( "%c", &ch ) ; // 讀進剛剛nextChar的東西
        scanStr[i] = ch ;
        gColumn ++ ;
        i ++ ;
        
        nextChar = cin.peek() ; // 再偷看下一個，若不是一樣的類別就跳出來，不要讀他
    } // while() assert : 這個while結束後，就有一個token(case1)了，要開始設定此token的資料
    
    // 若字串是END_OF_FILE就設eof為true結束讀的部分
    if ( strcmp( scanStr, "END_OF_FILE" ) == 0 ) {
        eof = true ;
    } // if()
    else {
      // assert:此時已經坐好了一個字串，還尚未進GetToken()
      tokenName = GetToken( scanStr ) ;
      // assert:此時做好了字串
    
      // 若此token(目前是CharPtr）存在就更新資料
      if ( TokenHasExist( tokenName, tokenPtr ) ) {
          UpdateToken( tokenName, tokenPtr ) ;
      } // if()
    
      // 此token(目前是CharPtr)不存在就新增
      else {
          AddNewToken( tokenName, IDENTIFIER, gFront ) ;
      } // else()
    } // else()

    gColumn = gColumn + strlen( scanStr ) ;
} // ProcessIdentifier()

// 處理Case2
void ProcessNumberAndString( char ch, TokenPtr &tokenPtr ) {
    Str100 scanStr = "\0" ;
    CharPtr tokenName = NULL ;
    int i = 0 ; // 用來計算scanStr中是第幾個index
    char nextChar = '\0' ;
    nextChar = cin.peek() ; // 先看第一個尚未讀進的字元是什麼，若也是a~z||A~Z||_則繼續讀
    scanStr[i] = ch ;
    i ++ ;
    
    while ( nextChar == '.' || ( nextChar >= '0' && nextChar <= '9' ) || nextChar == '\"' ) {
        // 若又讀到'\"'就代表字串結束了，要準備跳出去
        scanf( "%c", &ch ) ;
        scanStr[i] = ch ;
        i ++ ;
        
        nextChar = cin.peek() ; // 再偷看下一個，若不是一樣的類別就跳出來，不要讀他
    } // while()
    
    // assert:此時已經坐好了一個字串，還尚未進GetToken()
    tokenName = GetToken( scanStr ) ;
    // assert:此時做好了字串
    
    // 若此token(目前是CharPtr）存在就更新資料
    if ( TokenHasExist( tokenName, tokenPtr ) ) {
        UpdateToken( tokenName, tokenPtr ) ;
    } // if()
    
    // 此token(目前是CharPtr)不存在就新增
    else {
        AddNewToken( tokenName, CONSTANT, gFront ) ;
    } // else()
    
    
    gColumn = gColumn + strlen( scanStr ) ;
} // ProcessNumberAndString()

// 處理Case3
void ProcessSpecialSign( char ch, TokenPtr &tokenPtr ) {
    // Case3:特殊符號(以下所列出來的37種特殊符號)
    //       +   -   *   /   >   <   =   <=   >=   ==   !=   ?   %   &   &&
    //       ||   |   ^   "   '   .   ,   (   )   [   ]   {   }   <<   >>
    //       !   :   ;   #   ->  ++ --
    // 若是'<'則要偷看後面是什麼，若是 ＝ 或 < 要另外成一個token
    // 若是'>'則要偷看後面是什麼，若是 = 或 > 要另外成一個token
    // 若是'!'則要偷看後面是什麼，若是 = 要另外成一個token
    // 若是'&'則要偷看後面是什麼，若是 & 要另外成一個token
    // 若是'|'則要偷看後面是什麼，若是 | 要另外成一個token
    // 若是'-'則要偷看後面是什麼，若是 > 或 - 則要另外成一個token
    // 若是'+'則要偷看後面是什麼，若是 + 則要另外成一個token
    
    Str100 scanStr = "\0" ;
    CharPtr tokenName = NULL ;
    char nextChar = '\0' ;
    nextChar = cin.peek() ; // 先看第一個尚未讀進的字元是什麼，若也是a~z||A~Z||_則繼續讀
    scanStr[0] = ch ;
    
    if ( nextChar == '+' || nextChar == '-' || nextChar == '*' || nextChar == '/' || nextChar == '>' || nextChar == '<' || nextChar == '='
           || nextChar == '!' || nextChar == '?' || nextChar == '%' || nextChar == '&' || nextChar == '|' || nextChar == '^'
           || nextChar == '\"' || nextChar == '\'' || nextChar == '.' || nextChar == ',' || nextChar == '(' || nextChar == ')'
           || nextChar == '[' || nextChar == ']' || nextChar == '{' || nextChar == '}' || nextChar == ':' || nextChar == ';' || nextChar == '#' ) {
        // 先判斷特殊情況7種
        if ( ch == '<' && ( nextChar == '=' || nextChar == '<' ) ) {
            scanf( "%c", &ch ) ;
            scanStr[1] = ch ;
            // assert:此時字串長這樣{<,=,....}或{<,<,....}
        } // if()
        else if ( ch == '>' && ( nextChar == '=' || nextChar == '>' ) ) {
            scanf( "%c", &ch ) ;
            scanStr[1] = ch ;
        } // else if()
        else if ( ch == '!' && nextChar == '=' ) {
            scanf( "%c", &ch ) ;
            scanStr[1] = ch ;
        } // else if()
        else if ( ch == '&' && nextChar == '&' ) {
            scanf( "%c", &ch ) ;
            scanStr[1] = ch ;
        } // else if()
        else if ( ch == '|' && nextChar == '|' ) {
            scanf( "%c", &ch ) ;
            scanStr[1] = ch ;
        } // else if()
        else if ( ch == '-' && ( nextChar == '>' || nextChar == '-' ) ) {
            scanf( "%c", &ch ) ;
            scanStr[1] = ch ;
        } // else if()
        else if ( ch == '+' && nextChar == '+' ) {
            scanf( "%c", &ch ) ;
            scanStr[1] = ch ;
        } // else if()
        
        // 若是"//"則要把至換行前的東西讀掉
        else if ( ch == '/' && nextChar == '/' ) {
            // 若遇到換行就掉出來
            while ( ch != '\n' ) {
                scanf( "%c", &ch ) ; // 把nextChar獨進來
                gColumn ++ ;
            } // while()
            
            gLine ++ ;
            gColumn = 1 ;
            
            // assert:註解都不算，不需要轉乘token所以直接退出
            return ;
            
            // 跳出來後line要+1,column = 1
        } // else if()
        
        // 若是"/*"則要讀到"*/"為止
        else if ( ch == '/' && nextChar == '*' ) {
            // 要讀到"*/"才能停，所以需要用到cin.peek
            while ( ch == '*' && nextChar == '/' ) {
                scanf( "%c", &ch ) ;
                gColumn ++ ;
                nextChar = cin.peek() ;
                
                if ( ch == '\n' ) {
                    gLine ++ ;
                    gColumn = 1 ;
                } // if()
                
            } // while()
            
        } // else if()
        // 若都不是上述情況就直接開始做token
        
    } // if()
    
    // assert:此時已經坐好了一個字串，還尚未進GetToken()
    tokenName = GetToken( scanStr ) ;
    // assert:此時做好了字串
    
    // 若此token(目前是CharPtr）存在就更新資料
    if ( TokenHasExist( tokenName, tokenPtr ) ) {
        UpdateToken( tokenName, tokenPtr ) ;
    } // if()
    
    // 此token(目前是CharPtr)不存在就新增
    else {
        AddNewToken( tokenName, SPECIAL, gFront ) ;
    } // else()
    
    
    gColumn = gColumn + strlen( scanStr ) ;
} // ProcessSpecialSign()

void Read() {
    char ch = '\0' ;
    bool eof = false ;
    
    do {
        scanf( "%c", &ch ) ;
        
        // Case1:IDENTIFIER(底線或英文字母開頭，之後跟著英文字母或底線或數字)
        // 如果判斷到是這個分類，就要繼續讀直到空白，然後放成字串
        if ( ch == '_' || ( ch >= 'A' && ch <= 'Z' ) || ( ch >= 'a' && ch <= 'z' ) ) {
            ProcessIdentifier( ch, gFront, eof ) ;
        } // if()
        
        // Case2:數字 與 字串(數字可能會有小數點，字串(被2個"夾起來 不跨行))
        else if ( ( ch >= '0' && ch <= '9' ) || ch == '\"' ) {
            ProcessNumberAndString( ch, gFront ) ;
        } // else if()
        
        // Case3:特殊符號(以下所列出來的37種特殊符號)
        //       +   -   *   /   >   <   =   <=   >=   ==   !=   ?   %   &   &&
        //       ||   |   ^   "   '   .   ,   (   )   [   ]   {   }   <<   >>
        //       !   :   ;   #   ->  ++ --
        else if ( ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '>' || ch == '<' || ch == '=' || ch == '!' || ch == '?' || ch == '%'
                 || ch == '&' || ch == '|' || ch == '^' || ch == '\"' || ch == '\'' || ch == '.' || ch == ',' || ch == '(' || ch == ')'
                 || ch == '[' || ch == ']' || ch == '{' || ch == '}' || ch == ':' || ch == ';' || ch == '#' ) {
            ProcessSpecialSign( ch,  gFront ) ;
        } // else if()
        
        // 如果遇到white-space(空白, Tab, 換行)的話要將scanStr初始化
        else if ( ch == '\0' || ch == '\t' || ch == '\n' ) {
            
            if ( ch == '\0' ) {
                gColumn ++ ;
            } // if()
            else if ( ch == '\t' ) {
                gColumn += 2 ;
            }
            else if ( ch == '\n' ) {
                gLine ++ ;
                gColumn = 1 ;
            } // else if()
                
        } // else if()
    } while( !eof ) ;
    
    // assert:若eof=true則是遇到END_OF_FILE要結束讀取
    
} // Read()

void PrintOptionOfInteraction() {
    printf( "歡迎使用互動式功能選項\n" ) ;
    printf( "1.總共有多少種 token\n" ) ;
    printf( "2.三種case各有多少 token\n" ) ;
    printf( "3.印出指定 token 的位置 (line number,column number) (要排序)\n" ) ;
    printf( "4.印出所指定的那一 line 出現哪幾個 token (要排序)\n" ) ;
    printf( "5.結束\n" ) ;
} // PrintOptionOfInteraction()

// 總共有多少token
void ProcessInteraction1() {
    TokenPtr walkCount = NULL ;
    int tokenTotal = 0 ;
    
    walkCount = gFront ;
    
    while ( walkCount != NULL ) {
        tokenTotal ++ ;
        walkCount = walkCount -> next ;
    } // while()
    
    printf( "總共%d種\n", tokenTotal ) ;
    
} // ProcessInteraction1()

void CountWhiteSpace( int &whiteSpaceTotal, int number ) {
    whiteSpaceTotal = 0 ;
    Str100 numToString = "" ;
    sprintf( numToString, "%d", number ) ;
    whiteSpaceTotal = 5 - strlen( numToString ) ;
} // CountWhiteSpace()

// 三種case各有多少
//Case1 共    5個               // 這裡輸出的『共』
//Case2 共    2個               // 之後空格加數字共要5個字元
//Case3 共    8個               // 之後才接上『個』
void ProcessInteraction2() {
    TokenPtr walk = NULL ;
    int case1Total = 0 ;
    int case2Total = 0 ;
    int case3Total = 0 ;
    int whiteForCase1 = 0 ;
    int whiteForCase2 = 0 ;
    int whiteForCase3 = 0 ;
    
    walk = gFront ;
    while ( walk != NULL ) {
        if ( walk -> type == IDENTIFIER )
            case1Total ++ ;
        else if ( walk -> type == CONSTANT )
            case2Total ++ ;
        else if ( walk -> type == SPECIAL )
            case3Total ++ ;
        
        walk = walk -> next ;
    } // while()
    
    // assert:此時已經算好各個case的總數
    
    // 要用個function計算空白處
    CountWhiteSpace( whiteForCase1, case1Total ) ;
    CountWhiteSpace( whiteForCase2, case2Total ) ;
    CountWhiteSpace( whiteForCase3, case3Total ) ;
    
    // 到時候空白要用回印
    printf( "Case1 共" ) ;
    for ( int i = 1 ; i <= whiteForCase1 ; i ++ )
        printf( " " ) ;
    printf( "%d個\n", case1Total ) ;
    
    printf( "Case2 共" ) ;
    for ( int i = 1 ; i <= whiteForCase2 ; i ++ )
        printf( " " ) ;
    printf( "%d個\n", case2Total ) ;
    
    printf( "Case3 共" ) ;
    for ( int i = 1 ; i <= whiteForCase3 ; i ++ )
        printf( " " ) ;
    printf( "%d個\n", case3Total ) ;
    
} // ProcessInteraction2()

// 印出指定 token 的位置 (line number,column number) (要排序)
// 若無此token，輸出"查無此token: k"
void ProcessInteraction3() {
    Str100 tokenPreparedToFind = "" ;
    CharPtr tokenWeNeed = NULL ;
    TokenPtr current = NULL ;
    LinePtr lineWalk = NULL ;
    ColumnPtr columnWalk = NULL ;
    
    printf( "請輸入要搜尋的 Token : " ) ;
    scanf( " %s", tokenPreparedToFind ) ;
    tokenWeNeed = GetToken( tokenPreparedToFind ) ;
    if ( TokenHasExist( tokenWeNeed, gFront ) ) {
        current = FindToken( tokenWeNeed, gFront ) ;
        
        for ( lineWalk = current -> firstAppearOn ; lineWalk != NULL ; lineWalk = lineWalk -> next )
            for ( columnWalk = lineWalk -> firstAppearAt ; columnWalk != NULL ; columnWalk = columnWalk -> next )
                printf( "( %d, %d ) ", lineWalk -> line, columnWalk -> column ) ;
        
        printf( "\n" ) ;
    } // if()
    else
        printf( "查無此tokenn: %s\n", tokenPreparedToFind ) ;
    
} // ProcessInteraction3()

// 印出所指定的那一 line 出現哪幾個 token (要排序)
void ProcessInteraction4() {
    int lineWeNeed = -1 ;
    TokenPtr tokenWalk = NULL ;
    LinePtr lineWalk = NULL ;
    printf( "請輸入要指定的 line : " ) ;
    
    for ( tokenWalk = gFront ; tokenWalk != NULL ; tokenWalk = tokenWalk -> next ) {
        for ( lineWalk = tokenWalk -> firstAppearOn ; lineWalk != NULL ; lineWalk = lineWalk -> next ) {
            if ( lineWalk -> line == lineWeNeed )
                printf( "%s\n", tokenWalk -> tokenStr ) ;
        } // for()
    } // for()
    
} // ProcessInteraction4()

// ----------------------------------------main function-------------------------------------------------------

int main() {
    
    int numForInteraction = 0 ;
    
    Read() ; // 開始讀Input的資料
    
    // 讀完之後開始進入互動式介面
    PrintOptionOfInteraction() ;
    // assert:印出選項後開始用迴圈讀互動式選項並處理直到讀到5
    
    scanf( " %d", &numForInteraction ) ;
    while ( 1 ) {
        
        if ( numForInteraction == 1 ) {
            ProcessInteraction1() ;
        } // if()
        else if ( numForInteraction == 2 ) {
            ProcessInteraction2() ;
        } // else if()
        else if ( numForInteraction == 3 ) {
            ProcessInteraction3() ;
        } // else if()
        else if ( numForInteraction == 4 ) {
            ProcessInteraction4() ;
        } // else if()
        else if ( numForInteraction == 5 ) {
            printf( "byebye" ) ;
            return 0 ; // 結束程式
        } // else if()
        
        // 若是非1~5的數字
        else {
            printf( "請輸入數字 1 ~ 5\n" ) ;
        } // else()
        
        scanf( " %d", &numForInteraction ) ;
    } //while()
    
} // main()

