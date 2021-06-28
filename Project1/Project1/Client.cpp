#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <map>
#include <list>
#include <vector>
#include <SFML/Network.hpp>

unsigned short puertoBSS = 55555;
bool okConexion;
enum HEAD {
	CONEXION, DESCONEXION, LISTA, CHAT, UNIRME, CREARSALA, ASKSALA, SALACONFIRM, ASKNAME, NEWPLAYER, STARTGAME,
	PEDIRSALAS, PEDIRFILTROSALAS, LISTASALAS, MANO, READY, YOURTURN, NEXTTURN, PREGUNTA, INFO, DARCARTA, NOCARTA,
	CAMBIODETURNO, COMPROBARFAMILIAS, PEDIRSALASABIERTAS, JUGADORDESCONECTADO, NEWID, FINALPARTIDA
};

std::map<int, sf::TcpSocket*> jugadores;
sf::TcpSocket* _BSS;

//enum comando { INIT, CONNECT, LISTEN, LISTENSV, READY, REPARTIR, CRIMEN, DRAW };


int counter = 0;

//INFO Sala
std::string nombreSala;
std::string Password;
int numPlayers;
int miID;
int numCartas;
int puntos = 0;
bool partida = true;

enum tipo {
	ARABE,
	BANTU,
	CHINA,
	ESQUIMAL,
	INDIA,
	MEXICANA,
	TIROLESA
};



////enum para el lobby
//enum LOBBY {
//	CREARSALA, UNIRMESALA, SALIR
//};

enum estados { TURNO, WAIT };

sf::Packet packet;
class carta
{
public:
	tipo cartatipo;
	std::string nombre;

	carta(tipo t, std::string s)
	{
		cartatipo = t;
		nombre = s;

	}
private:

};
struct PlayerInfo
{
public:

	std::string name;
	std::list<carta> mano;
	int id;
	PlayerInfo();
	~PlayerInfo();
};

std::vector<carta> mano;
struct Player
{
	PlayerInfo info;
	sf::TcpSocket* sock;

	Player()
	{

	};

	Player(PlayerInfo _info, sf::TcpSocket* _sock)
	{
		info = info;
		sock = _sock;
	};
};
std::list<sf::TcpSocket*> clients;

struct peer
{
	sf::IpAddress ip;

	unsigned short puerto;
};
//class BSS
//{
//private:
//
//public:
//	std::map<int,std::list<sf::TcpSocket*>> salas;
//};

std::vector<peer> peers;
std::list<carta> baraja;
void enviar(sf::TcpSocket* sock, sf::Packet pack)
{
	sf::Socket::Status status;
	do
	{
		status = sock->send(pack);
		//std::cout << sock << ",  Puerto: " << sock->getRemotePort() << std::endl;
		//std::cout << "enivado\n";
	} while (status == sf::Socket::Status::Partial);
	pack.clear();


}

void updatePeers(sf::Packet _pk)
{
	int size;
	int id;
	std::string ip;
	unsigned short port;
	clients.clear();
	_pk >> size;
	for (size_t i = 0; i < size; i++)
	{
		sf::TcpSocket* _sock = new sf::TcpSocket();
		_pk >> id
			>> ip
			>> port;
		_sock->connect(ip, port);
		jugadores.insert(std::pair<int, sf::TcpSocket*>(id, _sock));
	}

}
//BSS* host;
void crearSala()
{
	sf::Packet pack;

	std::cout << "Introduzca el nombre de la sala: " << std::endl;
	std::cin >> nombreSala;

	std::cout << "Introduzca la contraseña de la sala: " << std::endl;
	std::cin.ignore();
	std::getline(std::cin, Password);
	do
	{
		std::cout << "Introduzca el numero de jugadores (3 - 6) " << std::endl;
		std::cin >> numPlayers;
	} while (numPlayers < 3 || numPlayers > 6);
	pack << HEAD::CREARSALA
		<< nombreSala
		<< Password
		<< numPlayers;
	enviar(_BSS, pack);
	//std::cout << "Sala nueva enviada\n";
	pack.clear();
}
void pedirSalas()
{
	sf::Packet pack;
	pack << HEAD::PEDIRSALAS;
	enviar(_BSS, pack);
	pack.clear();
}
void filtrarSalas()
{
	int num = 0;
	sf::Packet pack;
	do
	{
		std::cout << "Introduzca el numero de jugadores (3 - 6) " << std::endl;
		std::cin >> num;
	} while (num < 3 || num > 6);
	pack << HEAD::PEDIRFILTROSALAS
		<< num;
	enviar(_BSS, pack);
	//std::cout << "Sala nueva enviada\n";
	pack.clear();
}
void filtroPassword()
{
	sf::Packet pack;
	pack << HEAD::PEDIRSALASABIERTAS;
	enviar(_BSS, pack);
	//std::cout << "Sala nueva enviada\n";
	pack.clear();
}
void uniraSala()
{
	std::string nombreSala;
	std::string contraseña;
	std::cout << "Intoduce el nombre de la sala a la que te quieres unir: \n";
	std::cin >> nombreSala;
	std::cout << "Intoduce la contraseña: \n";
	std::cin.ignore();
	std::getline(std::cin, contraseña);
	sf::Packet pack;
	pack << HEAD::UNIRME
		<< nombreSala
		<< contraseña;
	enviar(_BSS, pack);
}

void Menu()
{
	std::cout << "-Deseas crear una sala (n)\n-Unirte a una sala (u)\n-Filtrar sala por n de jugadores (f)\n-Ver las alas abiertas (a)\n-Ver todas las salas disponibles (t)" << std::endl;
	char chartmp;
	sf::Packet pack;

	std::cin >> chartmp;
	switch (chartmp)
	{
	case 'f':
		filtrarSalas();
		break;
	case 'u':
		uniraSala();
		break;
	case 'n':
		crearSala();
		break;
	case 'a':
		filtroPassword();
		break;
	case 't':
		pedirSalas();
		break;
	}
}

void filtrarSalasR(sf::Packet pack)
{
	std::cout << "Salas disponibles: \n";
	int nSalas;
	std::string nombre;
	pack >> nSalas;
	for (int i = 0; i < nSalas; i++)
	{
		pack >> nombre;
		std::cout << nombre << std::endl;
	}
	Menu();
}
std::string ConversorString(tipo tribu)
{
	if (tribu == tipo::ARABE)
	{
		return "Arabe";
	}
	else if (tribu == tipo::BANTU)
	{
		return "Bantu";
	}
	else if (tribu == tipo::CHINA)
	{
		return "China";
	}
	else if (tribu == tipo::ESQUIMAL)
	{
		return "Esquimal";
	}
	else if (tribu == tipo::INDIA)
	{
		return "India";
	}
	else if (tribu == tipo::MEXICANA)
	{
		return "Mexicana";
	}
	else if (tribu == tipo::TIROLESA)
	{
		return "Tirolesa";
	}
}
void confirmReady()
{
	bool ready = false;
	while (!ready)
	{
		std::cout << "Estas preparado para empezar la partida? (y/n)" << std::endl;
		char chartmp;
		std::cin >> chartmp;

		switch (chartmp)
		{
		case 'y':
			ready = true;
			break;
		case 'n':
			break;
		}
	}
	sf::Packet pack;
	pack << HEAD::READY;
	enviar(_BSS, pack);

}
void printMano()
{
	std::cout << "Tu mano es la siguiente:\n";
	for (int i = 0; i < numCartas; i++)
	{
		std::string tribu = ConversorString(mano[i].cartatipo);
		std::cout << " -" << mano[i].nombre << " " << tribu << std::endl;
	}
}


void comprobamosfamilias() {
	std::cout << "Comprobamemos si has juntado alguna familia \n" << std::endl;

	int puntosInit = puntos;

	for (int i = 0; i < 7; i++)
	{
		tipo t = (tipo)i;
		int count = 0;
		for (int i = 0; i < numCartas; i++)
		{
			if (mano[i].cartatipo == t)
			{
				count++;
			}
		}
		if (count == 6)
		{
			for (int i = 0; i < numCartas; i++)
			{
				if (mano[i].cartatipo == (tipo)t)
				{

					mano.erase(mano.begin() + i);
					numCartas--;
					i--;

				}

			}
			sf::Packet pack;
			//respuesta << HEAD::DARCARTA << ;
			std::cout << "+1 punto" << std::endl;
			puntos++;
			pack << HEAD::COMPROBARFAMILIAS << t << miID << nombreSala;
			enviar(_BSS, pack);
			pack.clear();
		}

	}

	if (puntosInit == puntos)
	{
		std::cout << "No has conseguido juntar ninguna familia" << std::endl;
	}
	else
	{
		std::cout << "Has conseguido juntar " << (puntos - puntosInit) << std::endl;
	}

}
void recibirMano(sf::Packet pack)
{


	pack >> numCartas;

	for (int i = 0; i < numCartas; i++)
	{
		int cartatipo;
		std::string nombre;

		pack >> cartatipo >> nombre;
		carta c((tipo)cartatipo, nombre);
		mano.push_back(c);

	}
	comprobamosfamilias();
	printMano();

	confirmReady();
}
void escogerSala(sf::Packet pack)
{
	std::cout << "Salas disponibles: \n";
	int nSalas;
	std::string nombre;
	pack >> nSalas;
	for (int i = 0; i < nSalas; i++)
	{
		pack >> nombre;
		std::cout << nombre << std::endl;
	}
	std::cout << "        ///////////////////////////// \n";
	Menu();
}
void startGame(sf::Packet pack)
{
	std::cout << "**********Empezamos partida**********" << std::endl;
	int player;
	pack >> player >> numPlayers >> miID >> nombreSala;
	std::cout << "Mi ID es el " << miID << std::endl;
	std::cout << "Turno del jugador: " << player << std::endl;
}
bool compararMiembro(std::string miembro)
{
	if (miembro == "Abuelo")
	{
		return true;
	}
	else if (miembro == "Abuela")
	{
		return true;
	}
	else if (miembro == "Padre")
	{
		return true;
	}
	else if (miembro == "Madre")
	{
		return true;
	}
	else if (miembro == "Hijo")
	{
		return true;
	}
	else if (miembro == "Hija")
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool compararTribu(std::string tribu)
{
	if (tribu == "Arabe")
	{
		return true;
	}
	else if (tribu == "Bantu")
	{
		return true;
	}
	else if (tribu == "China")
	{
		return true;
	}
	else if (tribu == "Esquimal")
	{
		return true;
	}
	else if (tribu == "India")
	{
		return true;
	}
	else if (tribu == "Mexicana")
	{
		return true;
	}
	else if (tribu == "Tirolesa")
	{
		return true;
	}
	else
	{
		return false;
	}

}
bool comprobarJugador(int ID)
{
	if (ID < numPlayers && ID != miID)
	{
		return true;
	}
	else
	{
		return false;
	}
}
tipo conversorEnum(std::string tribu)
{
	if (tribu == "Arabe")
	{
		return tipo::ARABE;
	}
	else if (tribu == "Bantu")
	{
		return tipo::BANTU;
	}
	else if (tribu == "China")
	{
		return tipo::CHINA;
	}
	else if (tribu == "Esquimal")
	{
		return tipo::ESQUIMAL;
	}
	else if (tribu == "India")
	{
		return tipo::INDIA;
	}
	else if (tribu == "Mexicana")
	{
		return tipo::MEXICANA;
	}
	else if (tribu == "Tirolesa")
	{
		return tipo::TIROLESA;
	}
}


//startTurn incluye la parte de preguntar
void startTurn()
{
	bool miembroCorrecto = false;
	bool tribuCorrecta = false;
	bool jugadorCorrecto = false;
	std::string miembro;
	std::string tribu;
	int jugadorPreguntado;
	tipo t;
	sf::Packet pack;
	std::cout << "*******Es tu turno*******\n";
	std::cout << "Que carta quieres pedir? \n";


	while (!miembroCorrecto)
	{
		std::cout << "Que miembro de la familia quieres? \n -Abuelo\n-Abuela\n-Padre\n-Madre\n-Hijo\n-Hija\n";
		std::cin >> miembro;
		miembroCorrecto = compararMiembro(miembro);

	}

	while (!tribuCorrecta)
	{
		std::cout << "De que tribu?  \n -Arabe\n-Bantu\n-China\n-Esquimal\n-India\n-Mexicana\n-Tirolesa\n";
		std::cin >> tribu;
		tribuCorrecta = compararTribu(tribu);

	}
	t = conversorEnum(tribu);
	while (!jugadorCorrecto)
	{
		std::cout << "A que jugador le quieres preguntar?\n";
		std::cout << "Lista de jugadores : \n";
		for (int i = 0; i < numPlayers; i++)
		{
			if (i != miID)
				std::cout << i << std::endl;
		}
		std::cin >> jugadorPreguntado;
		jugadorCorrecto = comprobarJugador(jugadorPreguntado);

	}

	pack << HEAD::PREGUNTA << nombreSala;
	pack << miembro << t << jugadorPreguntado << miID;
	enviar(_BSS, pack);
	pack.clear();

}
bool comprobarCarta(std::string miembro, int tribu)
{
	for (auto it = mano.begin(); it != mano.end(); it++)
	{
		if (it->cartatipo == (tipo)tribu)
		{
			if (it->nombre == miembro)
			{
				mano.erase(it);
				numCartas--;
				return true;
			}
		}

	}
	return false;
}
void responderPregunta(sf::Packet pack)
{
	//<< miembro << tribu << jugadorQuePregunta;
	sf::Packet respuesta;
	std::string miembro;
	int tribu;
	int jugadorQuePregunta;
	pack >> miembro >> tribu >> jugadorQuePregunta;

	if (comprobarCarta(miembro, tribu))
	{
		respuesta << HEAD::DARCARTA << miembro << tribu << jugadorQuePregunta << nombreSala;
		std::cout << "Mecachis me ha quitado una carta\n";
	}
	else
	{
		respuesta << HEAD::NOCARTA << jugadorQuePregunta << nombreSala;
		std::cout << "No la tengo (muy a mi pesar)\n";
	}
	enviar(_BSS, respuesta);
	pack.clear();
}
void recibirCarta(sf::Packet pack)
{
	std::string miembro;
	int tribu;
	pack >> miembro >> tribu;
	carta newCarta((tipo)tribu, miembro);
	mano.push_back(newCarta);
	numCartas++;
	std::cout << "Has recibido la carta que has pedido\n";
	comprobamosfamilias();
	printMano();
	startTurn();
}
void completarFamilia(sf::Packet pack)
{

}
void pasaTurno()
{
	std::cout << "No tenia la carta que he pedido\n";
	std::cout << "            ******Fin del turno******\n";
	sf::Packet pack;
	pack << HEAD::NEXTTURN << nombreSala;
	enviar(_BSS, pack);
}
void printInfo(sf::Packet pack)
{
	//INFO << miembro << tribu << jugadorQuePregunta << jugadorPreguntado;
	std::string info;

	pack >> info;


	std::cout << info << std::endl;;
}
void printCambioDeTurno(sf::Packet pack)
{
	int player;
	pack >> player;
	std::cout << "Turno del jugador " << player << std::endl;
}
void lanzarThread()
{
	std::thread turnThread(startTurn);
	turnThread.detach();
}
void waitJugadores()
{
	int numConexiones = 0;
	while (numPlayers != numConexiones)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));

	}
}
int añadirJugador(sf::Packet pack)
{
	int id;
	std::string IP;
	unsigned short port;
	pack >> id >> IP >> port;
	std::pair<int, sf::TcpSocket*> newPlayer;
	newPlayer.first = id;
	newPlayer.second = new sf::TcpSocket();
	newPlayer.second->connect(IP, port);
	jugadores.insert(newPlayer);



	std::cout << "Se ha conectado un nuevo jugador \n";
	return id;
}
void controlDesconexionPlayer(sf::Packet pack)
{
	int idJugadorDesconectado;
	int cartasExtra;
	pack >> idJugadorDesconectado;
	std::cout << "El jugador " << idJugadorDesconectado << " se ha desconectado\n";
	std::cout << "Has recibido una parte de sus cartas\n";
	pack >> cartasExtra;
	numCartas = numCartas + cartasExtra;
	for (int i = 0; i < cartasExtra; i++)
	{
		int cartatipo;
		std::string nombre;

		pack >> cartatipo >> nombre;
		carta c((tipo)cartatipo, nombre);
		mano.push_back(c);

	}
	comprobamosfamilias();
	printMano();
	numPlayers--;

}
void actualizarID(sf::Packet pack)
{
	pack >> miID;
	std::cout << "Mi nueva ID es: " << miID << std::endl;
}
void finalPartida(sf::Packet pack)
{
	int idGanador;
	pack >> idGanador;
	if (idGanador == miID)
	{
		std::cout << "********HAS GANADO***********\n";
	}
	else
	{
		std::cout << "*******Ha ganado el jugador " << idGanador << "********* Otra vez sera...\n";
	}
	sf::Packet p;
	p << HEAD::DESCONEXION;
	enviar(_BSS, p);
	partida = false;

}
void recibir(sf::TcpSocket* sock)
{
	while (partida)
	{
		int id_newPlayer = 100;
		sock->setBlocking(true);
		sf::Socket::Status status;
		sf::Packet pack;
		status = sock->receive(pack);
		//std::cout << sock << ",Recibo del Puerto: " << sock->getRemotePort() << std::endl;
		if (status == sf::Socket::Done)
		{
			int cabecera;
			std::string tmp;
			pack >> cabecera;
			switch (cabecera)
			{
			case HEAD::CONEXION:

			case HEAD::DESCONEXION:
				break;
			case HEAD::LISTA:

				updatePeers(pack);

				break;
			case HEAD::MANO:
				recibirMano(pack);
				break;
			case HEAD::LISTASALAS:
				escogerSala(pack);
				break;
			case HEAD::PEDIRFILTROSALAS:
				filtrarSalasR(pack);
				break;
			case HEAD::ASKSALA:
				std::cout << "El nombre de la sala ya esta cogido, por favor introduzca otro nombre\n";
				crearSala();
				break;
			case HEAD::ASKNAME:
				std::cout << "El nombre de la sala o la contraseña son incorrectas, por favor vuelva a intentarlo\n";
				uniraSala();
				break;
			case HEAD::SALACONFIRM:

				break;
			case HEAD::NEWPLAYER:
				id_newPlayer = añadirJugador(pack);
				//std::thread threadrecibir(recibir, newPlayer.second);
				break;
			case HEAD::STARTGAME:
				startGame(pack);
				break;

			case HEAD::YOURTURN:
				lanzarThread();
				//startTurn();
				break;
			case HEAD::PREGUNTA:
				responderPregunta(pack);
				break;
			case HEAD::DARCARTA:
				recibirCarta(pack);
				break;
			case HEAD::INFO:
				printInfo(pack);
				break;
			case HEAD::NOCARTA:
				pasaTurno();
				break;
			case HEAD::CAMBIODETURNO:
				printCambioDeTurno(pack);
				break;
			case HEAD::COMPROBARFAMILIAS:
				comprobamosfamilias();
				break;
			case HEAD::NEWID:
				actualizarID(pack);
				break;
			case HEAD::JUGADORDESCONECTADO:
				controlDesconexionPlayer(pack);
				break;
			case HEAD::FINALPARTIDA:
				finalPartida(pack);
				break;
			default:
				break;
			}
			//std::cout << cabecera << std::endl;
		}
		/*else if (status == sf::Socket::Disconnected)
		{
			std::cout << "Desconectado miau\n";
			break;
		}*/
		if (id_newPlayer != 100)
		{
			std::thread threadrecibir(recibir, jugadores[id_newPlayer]); //Abrimos un nuevo thread para que reciba los paquetes del jugador que se acaba de unir
			threadrecibir.detach();
			id_newPlayer = 100;
		}
		pack.clear();
	}
	//std::cout << "salgo porque quiero\n";
}


bool Cliente(sf::TcpSocket *_socket, std::string _ip, unsigned short _puerto) {

	//Connect a una ip: puerto
	sf::Socket::Status status;
	//std::cout << _ip << "   ,    " << _puerto << std::endl;
	status = _socket->connect(_ip, _puerto);

	if (status != sf::Socket::Done) {
		std::cout << "Error en el connect....cerrando el programa";
		return false;
	}
	else {
		std::cout << "Conexion valida" << std::endl;
		std::thread threadrecibir(recibir, _socket);
		threadrecibir.detach();
		return true;
	}

}


void Recepcion() {

	while (true)
	{
		/*for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{

		}
		if (_BSS != nullptr)
		{
			recibir(_BSS);
		}*/

	}
}

void Envio()
{
	while (true)
	{
		std::string str;
		std::getline(std::cin, str);
		sf::Packet pack;
		pack << HEAD::CHAT;
		pack << str;

		for (std::list<sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
		{
			enviar(*it, pack);
		}
		if (_BSS != nullptr)
		{
			enviar(_BSS, pack);
		}
		//std::cout << "enivado\n";
		pack.clear();
	}
}

void endGame()
{
	std::string input;

	std::cin.ignore();
	std::getline(std::cin, input);
}

//void repartirCartas()
//{
//	std::cout << "Barajo y reparto las cartas" << std::endl;
//	packet.clear();
//	int r;
//
//	for (; baraja.size() != 0;)
//	{
//		for (int i = 0; i < clients.size(); i++)
//		{
//			std::list<carta>::iterator it = baraja.begin();
//			//std::list<PlayerInfo>::iterator itP = listaPlayers.begin();
//
//			if (baraja.size() > 0)
//			{
//				r = rand() % baraja.size();
//				std::advance(it, r);
//				clients.mano.push_back(*it);
//				baraja.erase(it);
//			}
//		}
//	}
//
//}
int main() {
	sf::TcpSocket* sock = new sf::TcpSocket();
	_BSS = new sf::TcpSocket();
	//Pedimos si queremos ser cliente o servidor

		//Pedimos ip:port y pasar por parametro
		/*std::cout << "Introduce la ip....." << std::endl;
		std::string ip;
		std::getline(std::cin, ip);*/

		/*	std::cout << "Introduce el puerto....." << std::endl;
			unsigned short puerto;
			std::cin >> puerto;*/
	std::string ip = sf::IpAddress::LocalHost.toString();
	unsigned short puerto = puertoBSS;
	okConexion = Cliente(_BSS, ip, puerto);
	//std::cin >> c;

	//std::cin >> puerto;

	if (okConexion)
	{
		std::cout << "Conexion realizada" << std::endl;
		Menu();

		/*std::thread threadSend(Envio);
		threadSend.detach();*/

		Recepcion();

	}
	sock->disconnect();
	delete sock;
	endGame();
	return 0;
}
