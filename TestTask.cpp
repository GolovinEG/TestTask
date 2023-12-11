#include <SDL.h>
#include <SDL_image.h>
#include <string>
#include <list>
#include <cmath>
#include <iostream>
#include <fstream>
using namespace std;

//Stream count constant
const int STREAM_COUNT = 4;

//Graph texture file name
const string GRAPH_FILE = "quad.png";

//Input streams file name
const string DATA_FILE = "testtxt.txt";

//Screen dimension constants
const int SCREEN_WIDTH = 1150;
const int SCREEN_HEIGHT = 639;

//Graph reference points
const int CENTER[2] = { 498, 299 };
const int MAX_POINT_1[2] = { 498, 67 };
const int MAX_POINT_2[2] = { 862, 299 };
const int MAX_POINT_3[2] = { 498, 553 };
const int MAX_POINT_4[2] = { 155, 299 };

//Max signed graph values
const int MAX_GRAPH_VALUES[STREAM_COUNT] = { MAX_POINT_1[1] - CENTER[1], MAX_POINT_2[0] - CENTER[0], MAX_POINT_3[1] - CENTER[1], MAX_POINT_4[0] - CENTER[0] };

//Starts up SDL and creates window
bool init();

//Loads graph image
bool loadGraph();

//Reads all input and writes them into input strings
bool readAllInputs();

//Frees media and shuts down SDL
void close();

//Loads individual image as texture
SDL_Texture* loadTexture(string path);

//Read single input from input string, then remove it
string readSingleInput(int index);

//Parse input from string to double, changing decimal separator if needed
double parseInput(string input);

//Parse double input to int coordinates
void parseToCoordinates(double input[2], int coordinates[2], int quadrant);

//Draw a 4-pixel line between two points
void drawLine(int firstPoint[2], int secondPoint[2]);

//Peredicate for sorting relations lists
bool relation_peredicate(list<double> relation1, list<double> relation2);

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Current displayed texture
SDL_Texture* gTexture = NULL;

//Txt input file
ifstream gInputFile;

//Input streams from file
string gInputs[STREAM_COUNT] = { "", "", "", "" };

//Relation lists
list<list<double>> gRelations[STREAM_COUNT];

//Max values among input streams
double gMaxValue[STREAM_COUNT] = { NULL, NULL, NULL, NULL };

//Coordinate lists
list<int> gCoordinatesX[STREAM_COUNT];
list<int> gCoordinatesY[STREAM_COUNT];

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		cout << ("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			cout << "Warning: Linear texture filtering not enabled!\n";
		}

		//Create window
		gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			cout << ("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (gRenderer == NULL)
			{
				cout << ("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					cout << ("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
			}
		}
	}

	return success;
}

bool loadGraph()
{
	//Loading success flag
	bool success = true;

	//Load PNG texture
	gTexture = loadTexture(GRAPH_FILE);
	if (gTexture == NULL)
	{
		//Output error
		cout << "Failed to load texture image!\n";
		success = false;
	}

	return success;
}

bool readAllInputs()
{
	//Reading success flag
	bool success = true;

	//Open TXT input file
	gInputFile.open(DATA_FILE, ifstream::in);
	if (gInputFile.fail())
	{
		//Output error
		cout << "Failed to load input file!\n";
		success = false;
	}
	else
	{
		//Read inputs from file
		string input = "";
		while (getline(gInputFile, input))
		{
			for (int i = 0; i < STREAM_COUNT; i++)
			{
				if (gInputs[i] != "")
				{
					//Adds space after every input but the first
					gInputs[i] += " ";
				}
				//Find the tab separator to get input length
				size_t spacePos = input.find("\t");
				if (spacePos != string::npos)
				{
					//Write input into input string, then remove it from stream, as well as the tab separator
					gInputs[i] += input.substr(0, spacePos);
					input.erase(0, spacePos + 1);
				}
				else
				{
					gInputs[i] += input;
				}
			}
			if (gInputFile.fail())
			{
				//Output error
				cout << "File read error!\n";
				success = false;
				break;
			}
		}
	}

	return success;
}

void close()
{
	//Free loaded image
	SDL_DestroyTexture(gTexture);
	gTexture = NULL;

	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();

	//Close txt input file
	gInputFile.close();
}

SDL_Texture* loadTexture(string path)
{
	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		//Output error
		cout << ("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			//Output error
			cout << ("Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return newTexture;
}

string readSingleInput(int index)
{
	//Single input to return
	string singleInput = "";

	//Find the space separator to get input length
	int spacePos = gInputs[index].find(" ");

	//Read a single input and removes it from input string, as well as the space separator
	if (spacePos != string::npos)
	{
		singleInput = gInputs[index].substr(0, spacePos);
		gInputs[index].erase(0, spacePos + 1);
	}
	else
	{
		singleInput = gInputs[index];
		//Emptying the string completely is required to mark it as finished
		gInputs[index] = "";
	}

	return singleInput;
}

double parseInput(string input)
{
	//Parsed input to return
	double parsedInput = NULL;

	//Replace the comma separator with a dot
	int commaPos = input.find(",");
	if (commaPos != string::npos)
	{
		input.replace(commaPos, 1, ".");
	}
	//String to double conversion
	parsedInput = stod(input);

	return parsedInput;
}

void parseToCoordinates(double input[2], int coordinates[2], int quadrant)
{
	//Separate cases are required for odd and even numbered value streams
	if (quadrant == 0 || quadrant == 2)
	{
		//Gets percentage of max value by X from input value, converts it to decimal coordinates compared to the graph's canter
		double number = input[0] / gMaxValue[quadrant + 1] * MAX_GRAPH_VALUES[quadrant + 1];
		//Rounds decimal coordinates to int and attaches them to center
		coordinates[0] = round(number) + CENTER[0];
		//Repeat by Y
		number = input[1] / gMaxValue[quadrant] * MAX_GRAPH_VALUES[quadrant];
		coordinates[1] = round(number) + CENTER[1];	
	}
	else
	{
		//Gets percentage of max value by X from input value, converts it to decimal coordinates compared to the graph's canter
		double number = input[0] / gMaxValue[quadrant] * MAX_GRAPH_VALUES[quadrant];
		//Rounds decimal coordinates to int and attaches them to center
		coordinates[0] = round(number) + CENTER[0];
		//Repeat by Y
		if (quadrant == 1)
		{
			number = input[1] / gMaxValue[quadrant + 1] * MAX_GRAPH_VALUES[quadrant + 1];
			coordinates[1] = round(number) + CENTER[1];
		}
		else
		{
			number = input[1] / gMaxValue[0] * MAX_GRAPH_VALUES[0];
			coordinates[1] = round(number) + CENTER[1];
		}
	}
}

void drawLine(int firstPoint[2], int secondPoint[2])
{
	//Draw 4 1-pixel lines,slightly shifting start and end points, to form a 4-pixel line
	SDL_RenderDrawLine(gRenderer, firstPoint[0], firstPoint[1], secondPoint[0], secondPoint[1]);
	SDL_RenderDrawLine(gRenderer, firstPoint[0] + 1, firstPoint[1], secondPoint[0] + 1, secondPoint[1]);
	SDL_RenderDrawLine(gRenderer, firstPoint[0], firstPoint[1] + 1, secondPoint[0], secondPoint[1] + 1);
	SDL_RenderDrawLine(gRenderer, firstPoint[0] + 1, firstPoint[1] + 1, secondPoint[0] + 1, secondPoint[1] + 1);
}

bool relation_peredicate(list<double> relation1, list<double> relation2)
{
	//Sorts in acsending order of the first values of relations
	return (relation1.front() < relation2.front());
}

int main(int argc, char* args[])
{
	//Start up SDL and create window
	if (!init())
	{
		cout << "Failed to initialize!\n";
	}
	else
	{
		//Load media
		if (!loadGraph())
		{
			cout << "Failed to load media!\n";
		}
		else
		{
			//Open input file
			if (!readAllInputs())
			{
				cout << "Failed to open file!\n";
			}
			else
			{
				//Skip first line
				for (int i = 0; i < STREAM_COUNT; i++)
				{
					//Gets space possition
					int spacePos = gInputs[i].find(" ");

					//Removes first input from input string
					if (spacePos != string::npos)
					{
						gInputs[i].erase(0, spacePos + 1);
					}
				}
				//Reads and parses inputs and fills relation lists
				while (gInputs[0] != "")
				{
					double inputLine[STREAM_COUNT] = { NULL, NULL, NULL, NULL };
					for (int i = 0; i < STREAM_COUNT; i++)
					{
						inputLine[i] = parseInput(readSingleInput(i));
						if (i > 0)
						{
							//Write to relations lists
							gRelations[i - 1].push_back({ inputLine[i - 1], inputLine[i] });
							if (i == 3)
							{
								gRelations[3].push_back({ inputLine[3], inputLine[0] });
							}
						}
					}
				}

				//Sorts relation lists by first value, records max values, parses values to graph coordinates
				for (int i = 0; i < STREAM_COUNT; i++)
				{
					gRelations[i].sort(relation_peredicate);
					gMaxValue[i] = gRelations[i].back().front();
					if (i > 0)
					{
						while (!gRelations[i - 1].empty())
						{
							int coordinates[2] = { NULL, NULL };
							double input[2] = { gRelations[i - 1].front().front(), gRelations[i - 1].front().back() };
							parseToCoordinates(input, coordinates, i - 1);
							//Write values to coorditane lists, remove used values from relations lists
							gCoordinatesX[i - 1].push_back(coordinates[0]);
							gCoordinatesY[i - 1].push_back(coordinates[1]);
							gRelations[i - 1].pop_front();
						}
					}
					if (i == 3)
					{
						while (!gRelations[i].empty())
						{
							int coordinates[2] = { NULL, NULL };
							double input[2] = { gRelations[i].front().front(), gRelations[i].front().back() };
							parseToCoordinates(input, coordinates, i);
							//Write values to coorditane lists, remove used values from relations lists
							gCoordinatesX[i].push_back(coordinates[0]);
							gCoordinatesY[i].push_back(coordinates[1]);
							gRelations[i].pop_front();
						}
					}
				}

				//Main loop flag
				bool quit = false;

				//Event handler
				SDL_Event e;

				//Draw graph and output it to window
				//Clear screen
				SDL_RenderClear(gRenderer);

				//Render texture to screen
				SDL_RenderCopy(gRenderer, gTexture, NULL, NULL);

				//Draw graph lines
				for (int i = 0; i < STREAM_COUNT; i++)
				{
					//Get color by quadrant
					switch (i)
					{
					case 0:
						SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF); //Green
						break;
					case 1:
						SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF); //Red
						break;
					case 2:
						SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0xFF, 0xFF); //Blue
						break;
					case 3:
						SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0x00, 0xFF); //Yellow
						break;
					}
					//Connects all coordinates from the list for the given quadrant
					while (gCoordinatesX[i].size() > 1)
					{
						int firstPoint[2] = { gCoordinatesX[i].front(), gCoordinatesY[i].front() };
						//Remove coorditane values used for the first point of a line
						gCoordinatesX[i].pop_front();
						gCoordinatesY[i].pop_front();
						int secondPoint[2] = { gCoordinatesX[i].front(), gCoordinatesY[i].front() };
						drawLine(firstPoint, secondPoint);
					}
				}

				//Update screen
				SDL_RenderPresent(gRenderer);

				//While application is running
				while (!quit)
				{
					//Handle events on queue
					while (SDL_PollEvent(&e) != 0)
					{
						//User requests quit
						if (e.type == SDL_QUIT)
						{
							quit = true;
						}
					}
				}
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
