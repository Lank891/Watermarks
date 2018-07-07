//Done By Andrzej Litkowski

#include <opencv2/opencv.hpp>

#include <iostream>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <vector>

using namespace cv;
using namespace std;



int main(int argc, char** argv)
{
	if (argc < 2) //Jeżeli nie otworzono żadnych plików
	{
		std::cout << "Przeciagnij pliki na program.\n\n";
		system("pause");
		return 1;
	}

	//Jeżeli mamy obrazy
	string watermarkPath = argv[0]; //Ścieżka do watermarka
	replace(watermarkPath.begin(), watermarkPath.end(), '\\', '/'); //Zmiana znaku \ na / (format wczytywania)
	watermarkPath.erase(watermarkPath.size() - 3, 3); //Usuwa rozszerzenie "exe" (3 znaki)
	watermarkPath += "png"; //Dodaje rozszerzenie "png"
	Mat watermark = imread(watermarkPath, IMREAD_UNCHANGED); //Odczytuje zdjęcie
	cout << watermarkPath << ": watermark zaladowany.\n";

	string bckgPath = watermarkPath; //Ścieżka do backgroundu na podstawie ścieżki do watermarka
	bckgPath.erase(bckgPath.size() - 13, 13); // - "Watermark.png" = 13 znaków
	bckgPath += "Background.png"; //+nazwa pliku
	Mat background = imread(bckgPath, IMREAD_UNCHANGED);
	cout << bckgPath << ": tlo zaladowane.\n\n";

	cout << "Podaj wspolczynnik krycia watermaka (zalecane 1) (1.0 - max krycie, 0.0 - brak krycia): ";
	double krycie;
	cin >> krycie;
	cout << "\n";
	if (krycie < 0. || krycie > 1.) krycie = 0.8; //Musi być między 0.0, a 1.0 - domyślnie niech będzie 0.8

	string folderPath = watermarkPath; //Folder na output - żeby było prościej ścieżka do watermarka
	folderPath.erase(folderPath.size() - 13, 13); // - "Watermark.png" = 13 znaków
	folderPath += "Output"; //+ nazwa folderu

	//Wielkość watermarka
	int watY = watermark.rows;
	int watX = watermark.cols;

	//Wielkość tła
	int bckX = background.cols;
	int bckY = background.rows;

	//Wielkość zdjęcia i współrzędne startu nakładnia watermarka
	int imgX, imgY, startX, startY;


	//Pętla łącząca zdjęcia
	for (int picCounter = 1; picCounter < argc; picCounter++)
	{
		Mat image; //Pojemnik na obraz
		
		Mat output; //Obraz będący outputem

		string imgPath = argv[picCounter]; //Ścieżka do zdjęcia
		replace(imgPath.begin(), imgPath.end(), '\\', '/'); //Zamiana \ na / (format wczytywania)
		cout << picCounter << ": " << imgPath << "\n";

		string picName = imgPath.substr(imgPath.find_last_of('/')); //Nazwa zdjęcia - fragment ścieżki od ostatniego '/' włącznie do końca

		image = imread(imgPath, IMREAD_UNCHANGED);

		//Jeżeli nie wczytano obrazu
		if (image.empty())
		{
			cout << "Nie udalo sie otworzyc powyzszego zdjecia.\n" << endl;
			system("pause");
			cout << "\n\n";
			continue;
		}

		//Początkowo output = background
		output = background.clone();

		//Jeżeli obraz jest większy od tła, to trzeba przeskalować tło
		imgX = image.cols;
		imgY = image.rows;
		if (imgX > bckX || imgY > bckY)
		{
			cout << "Powyzsze zdjecie jest wieksze od tla, więc zostanie przeskalowane do jego wymiarów.\nWymiary tla: "\
				<< bckX << ", " << bckY << "; Poprzednie wymiary obrazu: " << imgX << ", " << imgY << ".\n";
			cout << "\n";
			resize(output, output, Size(imgX, imgY));
		}

		//Parametry kompresji
		vector<int> compression_params;
		compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
		compression_params.push_back(0);

		//Aktualne wymiary outputu
		int outX = output.cols;
		int outY = output.rows;

		//Miejsce, do którego ma się przyczepić do tła obraz
		startX = (outX - imgX) / 2;
		startY = (outY - imgY) / 2;

		bool imageWithAlpha = (image.channels() == 4); //Jeżeli obraz ma alfę (4 kanały), to trzeba zapisać kolor jako Vec4b, jeżeli nie - Vec3b

		for (int y = startY; y < startY + imgY; y++)
		{
			for (int x = startX; x < startX + imgX; x++)
			{
				if (imageWithAlpha)
				{
					Vec4b color = output.at<Vec4b>(Point(x, y));	//Pobiera kolor w punkcie z obrazka
					Vec4b imgColor = image.at<Vec4b>(Point(x - startX, y - startY)); //Pobiera odpowiedni kolor z obrazka
					double alpha = (double)imgColor[3] / 255; //Alfa jest od 0 do 255, więc dzielimy przez 255 i * krycie
					double beta = 1. - alpha;
					color[0] = imgColor[0] * alpha + color[0] * beta;
					color[1] = imgColor[1] * alpha + color[1] * beta;
					color[2] = imgColor[2] * alpha + color[2] * beta;
					output.at<Vec4b>(Point(x, y)) = color; //Ustawia piksel
				}
				else
				{
					Vec3b imgColor = image.at<Vec3b>(Point(x - startX, y - startY)); //Pobiera odpowiedni kolor z obrazka
					//Nie potrzeba alfy, bo obrazek jej nie ma - jest w pełni nakładany od razu na tło
					output.at<Vec4b>(Point(x, y)) = Vec4b(imgColor[0], imgColor[1], imgColor[2], 255); //Ustawia piksel
				}
			}
		}

		//Jeżeli output jest mniejszy od watermarka
		if (outX < watX || outY < watY)
		{
			cout << "Powyzszy obrazek (z tlem) jest mniejszy od watermarka, więc zostanie przeskalowany do jego wymiarów.\nPoprzednie wymiary obrazka: "\
				<< outX << ", " << outY << "; Wymiary watermarka: " << watX << ", " << watY << ".\n";
			cout << "\n";
			resize(output, output, Size(watX, watY)); //To zostanie przeskalowany do jego wymiarów
		}

		//Obliczenie pozycji, do której ma się przyczepić watermark - ma być na środku
		startX = (outX - watX) / 2;
		startY = (outY - watY) / 2;

		for (int y = startY; y < startY + watY; y++)
		{
			for (int x = startX; x < startX + watX; x++)
			{
				//Teraz zawsze będzie z alfą, bo tło ma być z alfą
				Vec4b color = output.at<Vec4b>(Point(x, y));	//Pobiera kolor w punkcie z obrazka
				Vec4b watColor = watermark.at<Vec4b>(Point(x - startX, y - startY)); //Pobiera odpowiedni kolor z watermarka
				double alpha = krycie * (double)watColor[3] / 255; //Alfa jest od 0 do 255, więc dzielimy przez 255 i * krycie
				double beta = 1. - alpha;
				color[0] = watColor[0] * alpha + color[0] * beta;
				color[1] = watColor[1] * alpha + color[1] * beta;
				color[2] = watColor[2] * alpha + color[2] * beta;
				output.at<Vec4b>(Point(x, y)) = color; //Ustawia piksel
			}
		}

		//Zapis pod poprzednią nazwą do folderu
		imwrite(folderPath + picName, output, compression_params);
		cout << "\n";
	}
	
	cout << "To juz wszystkie obrazy.\n\n";
	system("pause");

	return 0;
}