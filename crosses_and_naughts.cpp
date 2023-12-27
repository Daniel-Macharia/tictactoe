#include<windows.h>
#include<stdlib.h>
#include<list>
#include<vector>
#include<map>
#include<string>
#include<ctime>
#include<queue>
using std::queue;
using std::map;
using std::list;
using std::vector;
using std::string;

int menuId = 10;
int buttonID = 1;

int wLength = 500,wWidth = 500;
int btnLength = ((int) wLength / 3) - 2 ;
int btnWidth = ((int) wWidth / 3 ) - 2;

char mainMenuName[] = "menu";
char level[] = "level";
char help[] = "help";
char darkMode[] = "Dark Mode";
char exitApp[] = "exit";

HINSTANCE hInst;
HWND hMainWindow;

HBITMAP naught;
HBITMAP cross;
HBITMAP blank;
//pair<char*, HBITMAP> ai,user;


// 24 represents black while 5 represents white
HBRUSH backColor = (HBRUSH) 24; // the background color of the window is set to this color

struct position;
LRESULT CALLBACK windProc(HWND, UINT, WPARAM, LPARAM);
void invertWindowColor();
bool win();
position getPosition(int);


//describe the users
map<char*, HBITMAP> user;

//my message box
LRESULT CALLBACK messageProc(HWND, UINT, WPARAM, LPARAM);

void registerMessageBox( HINSTANCE hInstance)
{
	WNDCLASS dialog = {0};
	
	dialog.hbrBackground = (HBRUSH) COLOR_WINDOW;
	dialog.hInstance = hInstance;
	dialog.hCursor = LoadCursor(NULL, IDC_HAND);
	dialog.lpszClassName = "messageBox";
	dialog.lpfnWndProc = messageProc;
	
	RegisterClass(&dialog);
}

void createMessageBox(char* message, char* caption , char* lBtn = "OK", char* rBtn = "Quit" )
{
	
	HWND hMessageBox = CreateWindow( "messageBox", caption, WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU
	, 50, 50, 300, 300, hMainWindow, NULL, NULL, NULL);
	   
	//SetWindowText(hMessageBox, message);
	CreateWindow("static", message, WS_VISIBLE | WS_BORDER | WS_CHILD , 0, 50, 300, 150
	, hMessageBox, (HMENU) 50, NULL, NULL );
	
	CreateWindow("button", lBtn, WS_VISIBLE | WS_BORDER | WS_CHILD , 60, 210, 100, 30
	, hMessageBox, (HMENU) 50, NULL, NULL );
	
	CreateWindow("button", rBtn, WS_VISIBLE | WS_BORDER | WS_CHILD , 165, 210, 100, 30
	, hMessageBox, (HMENU) 55, NULL, NULL );
	
	EnableWindow(hMainWindow, false); //disable the main window whenever the dialog is opened

}

//position structure
struct position{
	int row;
	int col;
	
	position(int r = 0, int c = 0)
	{
		row = r;
		col = c;
	}
	
	position(position& val)
	{
		col = val.col;
		row = val.row;
	}
	
    bool operator==(position val)
	{
		return ( row == val.row && col == val.col); 
	}
	
	bool operator!=(position val)
	{
		return ( row != val.row || col != val.col); 
	}
};

	
class menu
{
	private:
		char name[20];
		 int menuID;
		HMENU hmenu;
		HMENU hsubmenu;
		list<menu> items;
	public:
		menu()
		{
			hmenu = CreateMenu();
			menuID = menuId;
		    menuId++;
		}
		
		HMENU getMenu()
		{
			return hmenu;
		}
		
		menu(char* menuName)
		{
			strcpy(name, menuName);
			menuID = menuId;
		    menuId++;
		}
		
		char* getMenuName()
		{
			return name;
		}
		
		void setMenuName(char* menuName)
		{
			strcpy(name, menuName);
		}
		
		void setHmenu(HMENU hparentMenu)
		{
			hmenu = hparentMenu;
		}
		
		void createMenu()
	    {
	    	if( items.size() == 0 )
	    	  AppendMenu(hmenu, MF_STRING, menuID, name);
	    	else
	    	  {
	    	  	hsubmenu = CreateMenu();
	    	  	
	    	  	AppendMenu(hmenu, MF_POPUP, (UINT_PTR) hsubmenu, name);
	    	  	
	    	  	list<menu>::iterator it = items.begin();
	    	  	 while( it != items.end() )
	    	  	 {
	    	  	 	it->setHmenu(hsubmenu);
	    	  	 	it->createMenu();
	    	  	 	
	    	  	 	it++;
				   }
				   
				   //add this item to the menu
				   AppendMenu(getMenu(), MF_STRING, (UINT_PTR) menuId, "New Game" ); //id no 18
				   SetMenu(hMainWindow, hmenu);
			  }
		}
		
		void addMenuItem(char* itemName)
		{
			menu newItem(itemName);
		    items.push_back(newItem);
		}
		
		HMENU getMenuItem( char* itemName) //returns the menu handle of the menu item you have specified
		{                                  //if the item is not in the invoking menu object, it returns 
			list<menu>::iterator it = items.begin(); // a handle to the invoking menu object
			while( it != items.end())
			{
				if(strcmp( it -> name, itemName) == 0)
				  return it-> hmenu;
				
				it++;
			}
			
			return hmenu;
		}
		
		void addSubMenuItem(char* menuItemName, char* subMenuItem)
		{
			
			
			list<menu>::iterator it = items.begin();
			while( it != items.end())
			{
				
				if( strcmp(it -> getMenuName() , menuItemName) == 0 )
				  it->addMenuItem(subMenuItem);
				  
				  it++;
			}
			
		}
		
};

struct button
{
		char type[7];
		char mark[10];
	    int butID;
		bool state;

		button()
		{
			strcpy(mark,"blank");
			strcpy(type, "button");
	
			butID = buttonID;
			state = true;
			buttonID++;
		}
		
		void changeState(char* player)
		{
			strcpy(mark,  (user[player] == naught) ? "naught" : "cross");
			state = false;
	
			strcpy(type, "static");

		}
		
		
};

class mainWindow
{
	private:
		WNDCLASS wc;
		char className[20] ;
		button space[9];
		menu menus;
	
		//MSG msg;
	    DWORD style = WS_VISIBLE | WS_OVERLAPPED | WS_SYSMENU ;
		RECT canvas;
		
	    HWND btnHandle[9];
	    int mode = -1;// -1 for white mode,0 for dark mode 
				
	public:
		mainWindow() 
		{
			strcpy(className,  "Tic-Tac-Toe");
			loadImage();
			
		    canvas.left = 10;
		    canvas.top = 10;
		    canvas.right = canvas.left + wWidth;
		    canvas.bottom = canvas.top + wLength + 50;//added 50 to adjust the canvas
		    
		    //AdjustWindowRect(&canvas, style, false); 
			   
			registerWindow();
			createWindow();
		}
		
		HWND getHandle(int i)
		{
			return btnHandle[i];
		}
		
	
		void registerWindow()
		{
			//WNDCLASS wc ;
			
		//	wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
			wc.hbrBackground = backColor;
			wc.hCursor = LoadCursor(NULL, IDC_ARROW);
			wc.hInstance = hInst;
			wc.lpszClassName = className;
			wc.lpfnWndProc = windProc;
			
			RegisterClass(&wc) ;
			registerMessageBox( hInst);
			
		}
		
		void createWindow()
		{
			hMainWindow = CreateWindow(className, className,style, canvas.left, canvas.top, 
			canvas.right - canvas.left, canvas.bottom - canvas.top, NULL, NULL, hInst, NULL);
			
				addMenus();
				addControls();
				
				//ask the user's preference at creation and each time  a new game is started
					int userPref = MessageBox(hMainWindow,"Wanna use crosses?", "choose your card", MB_YESNO);
					if( userPref == IDYES)//if yes IDYES = 6 while IDNO = 7
					{
						user["USERopponent"] = cross;
						user["AIopponent"] = naught;
					}
					else
					{
						user["USERopponent"] = naught;
						user["AIopponent"] = cross;
					}
			
		}
		
		bool updateWindow( int btnID, char* player)
		{
			space[btnID].changeState(player);
			
		    DestroyWindow( btnHandle[btnID]);
			  
			addControls(btnID,player);
		
			return check(player);
		}
		
		void addMenus()
		{
			menus.setMenuName(mainMenuName);// id no 10
			menus.addMenuItem(level);//id no 11
			menus.addMenuItem(help);//id no 12
			menus.addMenuItem(darkMode);// id no 13
			menus.addMenuItem(exitApp);//id no 14
			
			//add submenu items to the menus
			menus.addSubMenuItem(level, "Easy"); //id no 15
			menus.addSubMenuItem(level, "Normal");//id no 16
			menus.addSubMenuItem(level, "Hard");// id no 17
			
		//	AppendMenu(menus.getMenu(), MF_STRING, (UINT_PTR) menuId, "New Game" ); //id no 18
			
			menus.createMenu();
		}
		
		void addControls(int btn = -1,char* player = NULL)
		{
			
			
			int x = 0;
			int y = 0;
			 
			 if(btn == -1)
			{
				for(int i = 0; i < 9; i++)
		    {
		    	switch( i % 3 )
		    	{
		    		case 0:
		    			x = 0;
		    		    break;
		    		case 1:
		    			x = 1 * ( wWidth / 3 ) + 1;
		    			break;
		    		default:
		    			x = 2 * ( wWidth / 3 ) + 1;
				}
				
				switch( i / 3 )
		    	{
		    		case 0:
		    			y = 0;
		    		    break;
		    		case 1:
		    			y = 1 * ( wLength / 3 ) + 1;
		    			break;
		    		case 2:
		    			y = 2 * ( wLength / 3 ) + 1;
				}
		    	
		    	btnHandle[i] = CreateWindow( space[i].type, "", WS_VISIBLE| WS_BORDER | WS_CHILD |
				( (strcmp(space[i].type,"static") == 0) ? SS_BITMAP : BS_BITMAP )
				, x, y, btnWidth, btnLength, hMainWindow, (HMENU) space[i].butID, NULL, NULL);
				
				SendMessage(btnHandle[i], ( strcmp(space[i].type,"static") == 0) ? STM_SETIMAGE : BM_SETIMAGE
				, IMAGE_BITMAP
				, (LPARAM) ( ( strcmp(space[i].mark,"blank") == 0) ? blank 
				: ( (strcmp(space[i].mark,"naught") == 0) ? naught : cross) ) );
				
				
			}
			
			}
			
			else{
				switch( btn % 3 )
		    	{
		    		case 0:
		    			x = 0;
		    		    break;
		    		case 1:
		    			x = 1 * ( wWidth / 3 ) + 1;
		    			break;
		    		default:
		    			x = 2 * ( wWidth / 3 ) + 1;
				}
				
				switch( btn / 3 )
		    	{
		    		case 0:
		    			y = 0;
		    		    break;
		    		case 1:
		    			y = 1 * ( wLength / 3 ) + 1;
		    			break;
		    		case 2:
		    			y = 2 * ( wLength / 3 ) + 1;
				}
		    	
		    	btnHandle[btn] = CreateWindow( space[btn].type, "", WS_VISIBLE| WS_BORDER | WS_CHILD |  SS_BITMAP
				, x, y, btnWidth, btnLength, hMainWindow, (HMENU) space[btn].butID, NULL, NULL);
				
				SendMessage(btnHandle[btn], STM_SETIMAGE , IMAGE_BITMAP, (LPARAM)user[player]);

			}
		}
		
		void loadImage( int flag = -1)
	    {
	    	int f = ( user["AIopponent"] == naught) ? 0 : 1;
	    	
	    	char imageType[3][20];
	    	if( flag == -1 )
			{
				strcpy(imageType[0], "naught.bmp");
				strcpy(imageType[1], "cross.bmp");
				strcpy(imageType[2], "blank.bmp");
			 } 
			 else
			 {
			 	strcpy(imageType[0], "invertedNaught.bmp");
				strcpy(imageType[1], "invertedCross.bmp");
				strcpy(imageType[2], "invertedBlank.bmp");
			 }
			 
	    	naught = (HBITMAP) LoadImage(NULL, imageType[0], IMAGE_BITMAP, btnLength, btnWidth, LR_LOADFROMFILE);
	    	cross = (HBITMAP) LoadImage(NULL, imageType[1], IMAGE_BITMAP, btnLength, btnWidth, LR_LOADFROMFILE);
	    	blank = (HBITMAP) LoadImage(NULL, imageType[2], IMAGE_BITMAP, btnLength, btnWidth, LR_LOADFROMFILE);
	    	
	    	//whenever called make sure to update what user uses
	    	( f == 0 ) ?  ( user["AIopponent"] = naught , user["USERopponent"] = cross)
			: ( user["AIopponent"] = cross, user["USERopponent"] = naught );
			
		}
		
		
		button* getButton(int butID)
		{
			return &space[butID];
		}
		
		friend bool win();
		
		bool isVector(button* b1, button* b2)
		{
			position p1(0, 0);
			position p2(0, 0);
			
			p1.row = ( getPosition(b1 -> butID )).row;
			p1.col = ( getPosition(b1 -> butID )).col;
			
			p2.row = ( getPosition(b2 -> butID )).row;
			p2.col = ( getPosition(b2 -> butID )).col;
			
			if( p1.row == p2.row)
			  return true;
			else
			 if( p1.col == p2.col)
			  return true;
			else
			 if( (p2.row - p1.row == 1 && p2.col - p1.col == 1) || (p2.row - p1.row == 2 && p2.col - p1.col == 2))
			   return true;
			else
			 if( (p2.row - p1.row == 1 && p2.col - p1.col == -1) || ( p2.row - p1.row == 2 && p2.col - p1.col == -2))
			   return true;
			else 
			 return false;
		}
		
		bool isVector(button* b1, button* b2, button* b3)
		{
			return ( isVector(b1, b2) && isVector(b2, b3) );
		}
		
		bool check(char* player)
		{
			button* hVals[3];
			button* vVals[3];
			button* btdVals[3];
			button* tbdVals[3];
			
			for( int r = 0; r < 3; r++)
			{
				for( int c = 0, i = 2; c < 3; c++, i--)
				{
					hVals[c] = getButton( r * 3 + c);
					vVals[c] = getButton( c * 3 + r);//to obtain the vertical matches
					
					//get pointers to button on diagonals
					if( r == c )
					  tbdVals[r] = getButton( r * 3 + c); //getButton() recieves an index to the button
					  
					if( r == i)
					  btdVals[r] = getButton( r * 3 + c);
				}
				//each time the inner loop finishes executing 
				//all vertical and horizontal pointers would have been collected
				if( strcmp(hVals[0] -> mark, "blank") != 0)
				if( isVector(hVals[0], hVals[1], hVals[2]) && strcmp(hVals[0] -> mark, hVals[1] -> mark) == 0
				 && strcmp(hVals[1] -> mark, hVals[2] -> mark) == 0)
				  {
				  
				  if( strcmp( player, "USERopponent") == 0)
				     createMessageBox( "You Win" , "GAME OVER", "New Game");
				  else
				     createMessageBox( "You Loose", "GAME OVER", "New Game");
				  
				  return true;
				  }
				
				if( strcmp(vVals[0] -> mark, "blank") != 0)
				if( isVector(vVals[0], vVals[1], vVals[2]) && strcmp(vVals[0] -> mark, vVals[1] -> mark) == 0
				 && strcmp(vVals[1] -> mark, vVals[2] -> mark) == 0)
				 {
				    
				  if( strcmp( player, "USERopponent") == 0)
				     createMessageBox( "You Win" , "GAME OVER", "New Game");
				  else
				     createMessageBox( "You Loose", "GAME OVER", "New Game");
				  
				  return true;
				 }
			}
			
			    if( strcmp(tbdVals[0] -> mark, "blank") != 0)
				if( isVector(tbdVals[0], tbdVals[1], tbdVals[2]) && strcmp(tbdVals[0] -> mark, tbdVals[1] -> mark) == 0
				 && strcmp(tbdVals[1] -> mark, tbdVals[2] -> mark) == 0)
				 {
				   
				  if( strcmp( player, "USERopponent") == 0)
				     createMessageBox( "You Win" , "GAME OVER", "New Game");
				  else
				     createMessageBox( "You Loose", "GAME OVER", "New Game");
				  
				  return true;
				  
				 }
				
				if( strcmp(btdVals[0] -> mark, "blank") != 0)
				if( isVector(btdVals[0], btdVals[1], btdVals[2]) && strcmp(btdVals[0] -> mark, btdVals[1] -> mark) == 0
				 && strcmp(btdVals[1] -> mark, btdVals[2] -> mark) == 0)
				  {
				  
				  if( strcmp( player, "USERopponent") == 0)
				     createMessageBox( "You Win" , "GAME OVER", "New Game");
				  else
				     createMessageBox( "You Loose", "GAME OVER", "New Game");
				   
				   return true;
				   }				
			
			return false;
		}
		
		
		void newGame()
		{
			
			for(int i = 0; i < 9; i++)
			{
				strcpy(space[i].mark, "blank");
				strcpy(space[i].type, "button");
				space[i].state = true;
				
				DestroyWindow(btnHandle[i]);
			}
			
			addControls();
			        
			        //ask for users preference each time a new game starts
			    	int userPref;
					userPref = MessageBox(hMainWindow,"Wanna use crosses?", "choose your card", MB_YESNO);
					if( userPref == IDYES)//if yes IDYES = 6 while IDNO = 7
					{
						user["USERopponent"] = cross;
						user["AIopponent"] = naught;
					}
					else
					{
						user["USERopponent"] = naught;
						user["AIopponent"] = cross;
					}
		}
		
	    void invertWindowColor()
		{	
			for( int i = 0; i < 9; i++)
			   DestroyWindow(btnHandle[i]);
			   
			//change mode 
			if( mode == -1)
			  {
			  	mode = 0;
			  }
			else
			  {
			  	mode = -1;
			  }
			   
			loadImage(mode);
			addControls();
			
		}
	
};


position getPosition(int btnID)
{
	btnID--;
	
    position pos;
    
	pos.row = btnID / 3;
	pos.col = btnID % 3;	
	
	return pos;
}

int getID(position pos)
{
	int ID = 1 + (pos.row * 3 + pos.col);
	
	return ID;
}

class agent //only maintains an internal state of the environment
{
	protected:
		string state[3][3];
		
	public:
		agent()
		{
			for(int r = 0; r < 3; r++)
			  for(int c = 0; c < 3; c++)
			    state[r][c] = "";
		}
		
		bool over()
		{
			bool full = true;
			for(int r = 0; r < 3; r++)
			 for(int c = 0; c < 3; c++)
			  if( state[r][c] == "blank")
			    full = false;
			    
			if( full == true)
			   MessageBox(NULL,"we draw", "GAME OVER",MB_OK);
			     
			return full;
		}
		
		void updateState(mainWindow& App)
		{
			position p;
		    for(int r = 0; r < 3; r++)
			  for(int c = 0; c < 3; c++)
			    {
			    	p.row = r;
			    	p.col = c;
			    	
			    	state[r][c] = ( App.getButton( getID(p) ) )-> mark;
				}
		}
		
		virtual void play(mainWindow& App)
		{
			updateState(App);
			bool found = false;
			
			int btnID = 0;
			position pos;
			while(!found && !over())
			{
				btnID = 1 + time(NULL) % 9;
				pos = getPosition(btnID);
				 if( state[pos.row][pos.col] == "blank")
				 {
				 	App.updateWindow(btnID,"AIopponent");
				 	found = true;
				 }
			}
			
			
		}
	
	
};

struct combination
{
	int first;
	int second;
	int third;
	
	combination()
	{
		first = second = third = 0;
	}
};

class goalAgent: public agent
{
	protected:
		vector<int> myMarks;
		list<combination> myPossibleWins;
		char myMark[20];
	public:
		
		goalAgent()
		{
			if( user["AIopponent"] == naught) //mark keeps track of the mark the AI is using
			 strcpy(myMark, "naught");
			 
			else
			 strcpy(myMark, "cross");
		}
		
		void updateMyMarks(mainWindow &App)
		{
			
			if( user["AIopponent"] == naught) //mark keeps track of the mark the AI is using
			 strcpy(myMark, "naught");
			else
			 strcpy(myMark, "cross");
			 
			updateState(App);
			
			myMarks.clear();
			
			position p;
			
			for( int r = 0; r < 3; r++)
			{
				for( int c = 0; c < 3; c++)
				{
					
						p.row = r;
						p.col = c;
					if( strcmp( App.getButton(getID(p) - 1)->mark, myMark) == 0)
						myMarks.push_back( getID(p));
					
				}
			}
		
		}
		
		void updateMyPossibleWins(mainWindow &App)
		{
			myPossibleWins.clear();
			
			combination a;
			
			vector<int>::iterator r,c;
			
			for( r = myMarks.begin(); r != myMarks.end(); r++)
			{
				for( c = myMarks.begin(); c != myMarks.end(); c++)
				{
					if( *r == *c) // you can't have a combination with the same button twice
					 continue;
					 
					if( App.isVector( App.getButton( *r - 1), App.getButton( *c - 1)) 
					 && strcmp( App.getButton( getMissing( *r, *c) - 1)->mark, "blank") == 0)
					{
						a.first = *r;
						a.second = *c;
						
						myPossibleWins.push_back(a);
					}
				}
				
			}
		
		}
		
		int getSecond(int ID, mainWindow &App)
		{
			int h[3], v[3], d1[3], d2[3];
			
			for( int r = 0, i = 2; r < 3; r++, i--)
			{
				for( int c = 0, j = 2; c < 3; c++, j--)
				{
					h[c] = r * 3 + c + 1;
					v[c] = c * 3 + r + 1;
					
					if( r == c)
					  d1[r] = r * 3 + c + 1;
					 
					 if( i == c)
			          d2[r] = r * 3 + c + 1;
					  
				}
				
				if( ID == h[0] ) // check horizontally
				{
					if( strcmp( App.getButton( h[1] - 1)->mark, "blank") == 0 
					  && strcmp( App.getButton( h[2] - 1)->mark, "blank") == 0 )
					    return h[2];
				}
				else
				 if( ID == h[1] )
				{
					if( strcmp( App.getButton( h[0] - 1)->mark, "blank") == 0 
					  && strcmp( App.getButton( h[2] - 1)->mark, "blank") == 0 )
					    return h[2];
				}
				else
				 if( ID == h[2] )
				{
					if( strcmp( App.getButton( h[0] - 1)->mark, "blank") == 0 
					  && strcmp( App.getButton( h[1] - 1)->mark, "blank") == 0 )
					    return h[0];
				}
				
				if( ID == v[0] ) // check vertically
				{
					if( strcmp( App.getButton( v[1] - 1)->mark, "blank") == 0 
					  && strcmp( App.getButton( v[2] - 1)->mark, "blank") == 0 )
					    return v[2];
				}
				else
				 if( ID == v[1] )
				{
					if( strcmp( App.getButton( v[0] - 1)->mark, "blank") == 0 
					  && strcmp( App.getButton( v[2] - 1)->mark, "blank") == 0 )
					    return v[2];
				}
				else
				 if( ID == v[2] )
				{
					if( strcmp( App.getButton( v[0] - 1)->mark, "blank") == 0 
					  && strcmp( App.getButton( v[1] - 1)->mark, "blank") == 0 )
					    return v[0];
				}
				
			}
			
				if( ID == d1[0] ) // check top-bottom diagonal
				{
					if( strcmp( App.getButton( d1[1] - 1)->mark, "blank") == 0 
					  && strcmp( App.getButton( d1[2] - 1)->mark, "blank") == 0 )
					    return d1[2];
				}
				else
				 if( ID == d1[1] )
				{
					if( strcmp( App.getButton( d1[0] - 1)->mark, "blank") == 0 
					  && strcmp( App.getButton( d1[2] - 1)->mark, "blank") == 0 )
					    return d1[2];
				}
				else
				 if( ID == d1[2] )
				{
					if( strcmp( App.getButton( d1[0] - 1)->mark, "blank") == 0 
					  && strcmp( App.getButton( d1[1] - 1)->mark, "blank") == 0 )
					    return d1[0];
				}
				
				if( ID == d2[0] ) // check bottom-top diagonal
				{
					if( strcmp( App.getButton( d2[1] - 1)->mark, "blank") == 0 
					  && strcmp( App.getButton( d2[2] - 1)->mark, "blank") == 0 )
					    return d2[2];
				}
				else
				 if( ID == d2[1] )
				{
					if( strcmp( App.getButton( d2[0] - 1)->mark, "blank") == 0 
					  && strcmp( App.getButton( d2[2] - 1)->mark, "blank") == 0 )
					    return d2[2];
				}
				else
				 if( ID == d2[2] )
				{
					if( strcmp( App.getButton( d2[0] - 1)->mark, "blank") == 0 
					  && strcmp( App.getButton( d2[1] - 1)->mark, "blank") == 0 )
					    return d2[0];
				}
			
			return -1;
		}
		
		int getMissing(int a, int b) //returns the id to the remaining button in the vector
       {
       	    int min = 0, max = 0;
	
	       ( b > a) ? (max = b, min = a) : (max = a, min = b);
	
	       int h[3], v[3], d1[3], d2[3];
	
        	for( int r = 0, j = 2; r < 3; r++,j--)
	        {
		       for( int c = 0; c < 3; c++)
		        { 
		        	h[c] = r * 3 + c + 1;
		        	v[c] = c * 3 + r + 1;
			
			        if( r == c)
			          d1[r] = r * 3 + c + 1;
			
			        if( j == c)
			          d2[r] = r * 3 + c + 1;
			  
		
		        }
		
			   //check horizontally
			   if( h[0] == min)
			   {
			    	if( h[1] == max)
				      return h[2];
				    else
				    if( h[2] == max)
				      return h[1];
			    }
			    else
			    if(  h[1] == min)
			    {
				   if( h[2] == max)
				     return h[0];
			    }
			
			    //now check vertically
			    if( min == v[0])
			    {
			 	    if( max == v[1])
			 	      return v[2];
			 	    else
			 	    if( max == v[2])
			 	      return v[1];
			    }
			    else
			      if( min == v[1])
			      {
			 	     if( max == v[2])
			 	        return v[0];
			      }	
		
	        }
	   
	        //check top-bottom
			if( min == d1[0])
			{
				if( max == d1[1])
				  return d1[2];
				else
				if(max == d1[2])
				  return d1[1];
			}
			else
			if( min == d1[1])
			{
				if(max == d1[2])
				  return d1[0];
			}
			
			//now check bottom-top
			 if( min == d2[0])
			 {
			 	if( max == d2[1])
			 	  return d2[2];
			 	else
			 	if( max == d2[2])
			 	  return d2[1];
			 }
			 else
			 if( min == d2[1])
			 {
			 	if( max == d2[2])
			 	 return d2[0];
			 }
			 
			 
			return -1;// when no missing is found return -1
	                   // remember no button has an ID of -1
        }
        
        void play(mainWindow &App)
        {
        	updateMyMarks(App);
        	updateMyPossibleWins(App);
        
        	
        	if( myPossibleWins.empty() == false )
        	{
        		int thirdButton;
        		
        		list<combination>::iterator it;
        		it = myPossibleWins.begin();
        		
        		while(it != myPossibleWins.end())
        		{
        			thirdButton = getMissing( it->first, it->second);
        			
        		    if( thirdButton == -1)
        		      continue;
        		    else
        		    {
        		    	App.updateWindow( thirdButton - 1, "AIopponent");
        		    	return;
					}
				}
			}
			
			if( myMarks.empty() == false)
			{
				int sec;
				combination a;
				
				for( int i = 0; i < myMarks.size(); i++)
				{
					sec = getSecond(myMarks[i], App);
					
					if( sec == -1)
					 continue;
					 
					else
					{
						App.updateWindow( sec - 1, "AIopponent");
						return;
					}
				}
			}
			
			return agent::play(App);
		}
		
};


//utility agent considers that incase there`s no possible win for it and there is one 
// for the opponent, then blocking that of the opponent would be a rational thing to do
//hence it keeps track of the opponent 

class utilityAgent : public goalAgent
{
	private:
		vector<int> opponentMarks;
		list<combination> opponentPossibleWins;
		char opponentMark[20];
		
	public:
		utilityAgent()
		{
			if( user["USERopponent"] == naught )
			  strcpy( opponentMark, "naught");
			else
			  strcpy( opponentMark, "cross");
			  
		}
		
		void updateOpponentMarks(mainWindow &App)
		{
			
			if( user["USERopponent"] == naught )
			  strcpy( opponentMark, "naught");
			else
			  strcpy( opponentMark, "cross");
			
			opponentMarks.clear();
			
			for( int i = 0; i < 9; i++)
			{
				if( strcmp( App.getButton(i)->mark, opponentMark ) == 0) // the button contains an opponent`s 
				{                                                        // mark
					opponentMarks.push_back( i + 1);
				}
			}
		
		}
		
		void updateOpponentPossibleWins( mainWindow &App)
		{
			opponentPossibleWins.clear();
			
			combination a;
			
			for( int i = 0; i < opponentMarks.size(); i++)
			{
				for( int j = 0; j < opponentMarks.size(); j++)
				{
	                
					if( i == j)
					  continue;
					
					a.first = opponentMarks[i];
					a.second = opponentMarks[j];
					  
					if( strcmp( App.getButton( getMissing( a.first , a.second) - 1 )->mark, "blank" ) == 0)
					{
						opponentPossibleWins.push_back(a);
					}
				}
				
			}
		
		}
		
		void play(mainWindow &App)
		{
			updateMyMarks(App);
			updateMyPossibleWins(App); //update here because well check the Agent`s marks as well
			
			updateOpponentMarks(App);
			updateOpponentPossibleWins(App);
			
			// if the opponent has a chance of winning and the AI has none then
			// the rational action to take is to block the opponent`s chance of winning
			if( myPossibleWins.empty() == true && opponentPossibleWins.empty() == false)
			{
				int third;
				list<combination>::iterator it;
				it = opponentPossibleWins.begin();
				
				do
				{
					third = getMissing( it->first, it->second);
					
					if( strcmp( App.getButton( third - 1)->mark, "blank") == 0)
					{
						App.updateWindow(third - 1, "AIopponent");
						return;
					}
					
					it++;
				}while( it != opponentPossibleWins.end() );
			}
			
			//if the opponent has not chance of winning then 
			//play aiming to win
			
			goalAgent::play(App);
			
		}
		
		
};

//the main function
mainWindow app;
agent* opponent = new agent;

queue<int> commandQueue;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR args, int ncmdArgs)
{
	hInst = hInstance;
	
	//mainWindow app;
	
	MSG msg;
	
		while( GetMessage( &msg, NULL, NULL, NULL) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
				
			}
	
	return 0;
}

	LRESULT CALLBACK windProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
		{
			switch( msg )
			{
				case WM_CLOSE:
					DestroyWindow(hwnd);
					//MessageBox(NULL,"Quit?","Nothing",MB_YESNO);
					PostQuitMessage(0);
					break;
				
				case WM_COMMAND:
					
				    commandQueue.push(wp);
				    
				    do
				    {
				    		switch( commandQueue.front() )//only the user interracts with the window directly
					{
						case 1:
							if( app.updateWindow(0,"USERopponent" ))
							   break;//when user wins there's not need of letting the AI play
							
			             	//before getting another message,let the AI opponent play
			            	opponent->play(app);
				
							break;
						
						case 2:
							if( app.updateWindow(1,"USERopponent" ))
							   break;//when user wins there's not need of letting the AI play
							
							//before getting another message,let the AI opponent play
			            	opponent->play(app);
				
							break;
						
						case 3:
							if( app.updateWindow(2,"USERopponent" ))
							   break;//when user wins there's not need of letting the AI play
							
							//before getting another message,let the AI opponent play
			            	opponent->play(app);
				
							break;
						
						case 4:
							if( app.updateWindow(3,"USERopponent" ))
							   break;//when user wins there's not need of letting the AI play
							
							//before getting another message,let the AI opponent play
			            	opponent->play(app);
				
							break;
						
						case 5:
							if( app.updateWindow(4,"USERopponent" ))
							   break;//when user wins there's not need of letting the AI play
							
							//before getting another message,let the AI opponent play
			            	opponent->play(app);
				
							break;
						
						case 6:
							if( app.updateWindow(5,"USERopponent" ))
							   break;//when user wins there's not need of letting the AI play
							
							//before getting another message,let the AI opponent play
			            	opponent->play(app);
				
							break;
						
						case 7:
							if( app.updateWindow(6,"USERopponent" ))
							   break;//when user wins there's not need of letting the AI play
							
							//before getting another message,let the AI opponent play
			            	opponent->play(app);
				
							break;
						
						case 8:
							if( app.updateWindow(7,"USERopponent" ))
							   break;//when user wins there's not need of letting the AI play
							
							//before getting another message,let the AI opponent play
			            	opponent->play(app);
				
							break;
						
						case 9:
							if( app.updateWindow(8,"USERopponent" ))
							   break;//when user wins there's not need of letting the AI play
							
							//before getting another message,let the AI opponent play
			            	opponent->play(app);
				
							break;
						
						
						//handle Menu Events
						case 12: //when help menu item is clicked
						    MessageBox(hwnd,
							"A player wins if he marks 3 squares either:\n vertically,horizontally or diagonally "
							, "HELP", MB_OK);
							break;
							
						case 13 ://when user chooses darkMode
						    app.invertWindowColor();
						    //createMessageBox("How may I help? ", "HELP", "OK", "CANCEL");
						    break;
						    
						case 14://when presses the exit button
							DestroyWindow(hMainWindow);
							PostQuitMessage(0);
							break;
							
						case 15: //when User switches to level "Easy"
						    MessageBox(hwnd, "Swithing to level 'Easy' ","Changing Level",MB_OK);
							delete opponent;
							opponent = new agent;
							break;
							
						case 16: //when User switches to level "normal"
						    MessageBox(hwnd, "Swithing to level 'Normal' ","Changing Level",MB_OK);
							delete opponent;
							opponent = new goalAgent;
							break;
							
						case 17: //when User switches to level "Hard"
						    MessageBox(hwnd, "Swithing to level 'Hard' ","Changing Level",MB_OK);
							delete opponent;
							opponent = new utilityAgent;
							break;
							
					    case 18: // when user clicks the new game menu item
					    	app.newGame();
					    	break;
						
					}
					
					  commandQueue.pop();//remove the processed command
					} while( !commandQueue.empty());
					
					break;
				
				default:
					return DefWindowProc(hwnd, msg, wp, lp);
			}
		}
	
	
	
LRESULT CALLBACK messageProc(HWND hwnd, UINT msg, WPARAM wp,  LPARAM lp)
{
	switch( msg)
	{
		case WM_COMMAND:
			
			switch( wp)
			{
				case 50:
					EnableWindow(hMainWindow, true);
					DestroyWindow(hwnd);
					app.newGame();
					break;
					
				case 55:
					EnableWindow(hMainWindow, true);
					PostQuitMessage(0);
					break;
			}
			
		    break;
		case WM_CLOSE:
			EnableWindow(hMainWindow, true);
			DestroyWindow(hwnd);
			
			//make all the buttons static
			for( int i = 0; i < 9; i++)
			{
				strcpy(( app.getButton(i)) -> type , "static");
				DestroyWindow( app.getHandle(i));
			}
			app.addControls();
			break;
			
		default:
			return DefWindowProc(hwnd, msg, wp, lp);
	}
}

