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

int playerTurn = 0;
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
enum tipo {
	ARABE,
	BANTU,
	CHINA,
	ESQUIMAL,
	INDIA,
	MEXICANA,
	TIROLESA
};

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

struct Sala
{
	std::string nombre;
	std::string password;
	int maxPlayers;
	int playersConected;
	int lastID;
	int playersReady = 0;
	std::map <int, sf::TcpSocket*> JugadoresSala;
	std::vector<std::vector<carta>> copiasManos;
	std::vector<int> puntos;

	bool partidaIniciada;
};

bool partida = true;

std::list<sf::TcpSocket*> clients;

std::map < std::string, Sala> Salas;

int numSalas;

std::vector <carta> baraja;
void crearbaraja()
{
	for (int t = 0; t < 7; t++)
	{
		for (int f = 0; f < 6; f++)
		{
			switch (f)
			{
			case 0: baraja.push_back(carta((tipo)t, "Abuelo"));
				break;
			case 1: baraja.push_back(carta((tipo)t, "Abuela"));
				break;
			case 2: baraja.push_back(carta((tipo)t, "Padre"));
				break;
			case 3: baraja.push_back(carta((tipo)t, "Madre"));
				break;
			case 4: baraja.push_back(carta((tipo)t, "Hijo"));
				break;
			case 5: baraja.push_back(carta((tipo)t, "Hija"));
				break;
			default:
				break;
			}
		}
	}
}
enum HEAD {
	CONEXION, DESCONEXION, LISTA, CHAT, UNIRME, CREARSALA, ASKSALA, SALACONFIRM, ASKNAME, NEWPLAYER, STARTGAME,
	PEDIRSALAS, PEDIRFILTROSALAS, LISTASALAS, MANO, READY, YOURTURN, NEXTTURN, PREGUNTA, INFO, DARCARTA, NOCARTA,
	CAMBIODETURNO, COMPROBARFAMILIAS, PEDIRSALASABIERTAS, JUGADORDESCONECTADO, NEWID, FINALPARTIDA
};

std::map<int, sf::TcpSocket*> jugadores;
sf::TcpSocket* _BSS;

//enum comando { INIT, CONNECT, LISTEN, LISTENSV, READY, REPARTIR, CRIMEN, DRAW };
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
void enviarFinalPartida(Sala s)
{
	sf::Packet pack;
	int idGanador;
	int puntosGanador = 0;
	for (int i = 0; i < s.playersConected; i++)
	{
		if (puntosGanador <= s.puntos[i])
		{
			puntosGanador = s.puntos[i];
			idGanador = i;
		}

	}
	//std::string msgGanador = 
	pack << HEAD::FINALPARTIDA << idGanador;
	for (int i = 0; i < s.playersConected; i++)
	{
		enviar(s.JugadoresSala[i], pack);
	}
	//partida = false;
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

void crearSala(sf::Packet _pack, sf::TcpSocket* _sock)
{
	std::string nombre;
	Sala tmp;
	std::pair<std::string, Sala> newSala;
	_pack >> nombre;
	std::cout << "Nombre recibido:  " << nombre << std::endl;
	std::cout << "Nombre Sala existente:  " << Salas[nombre].nombre << std::endl;
	if (nombre.compare(Salas[nombre].nombre) != 0)
	{
		std::cout << "No existe ninguna sala con ese nombre\n";

		tmp.nombre = nombre;
		_pack >> tmp.password;
		_pack >> tmp.maxPlayers;
		newSala.first = nombre;
		//newSala.second = tmp;
		Salas.insert(newSala);
		Salas[nombre].nombre = nombre;
		Salas[nombre].password = tmp.password;
		Salas[nombre].maxPlayers = tmp.maxPlayers;
		Salas[nombre].lastID = 0;
		Salas[nombre].playersConected = 1;
		std::pair<int, sf::TcpSocket*> newPlayer;
		newPlayer.first = Salas[nombre].lastID;
		newPlayer.second = _sock;
		Salas[nombre].JugadoresSala.insert(newPlayer);
		for (int i = 0; i < Salas[nombre].maxPlayers; i++)
		{
			std::vector<carta>mano;
			int p = 0;
			Salas[nombre].copiasManos.push_back(mano);
			Salas[nombre].puntos.push_back(p);
		}
		std::cout << "Nueva sala creada\n";
		numSalas++;
		_pack.clear();
		_pack << HEAD::SALACONFIRM;
		enviar(_sock, _pack);
	}
	else
	{
		std::cout << "ya existe una sala con ese nombre\n";
		_pack.clear();
		_pack << HEAD::ASKSALA;
		enviar(_sock, _pack);
	}
}
void repartirCartas(Sala &s)
{
	int cartasRestantes = 42;
	std::vector<std::vector<carta>>Manos;
	for (int i = 0; i < s.playersConected; i++) //Generamos las manos necesarias
	{
		std::vector<carta>mano;
		Manos.push_back(mano);
	}
	while (cartasRestantes > 0)   //Repartimos hasta quedarnos sin cartas
	{
		for (int i = 0; i < s.playersConected && cartasRestantes > 0; i++)
		{
			int azar = rand() % cartasRestantes;
			carta repartida(baraja[azar].cartatipo, baraja[azar].nombre);
			Manos[i].push_back(repartida);
			s.copiasManos[i].push_back(repartida);
			baraja.erase(baraja.begin() + azar);
			cartasRestantes--;
		}
		//std::cout << cartasRestantes << std::endl;
	}
	for (int i = 0; i < s.playersConected; i++)
	{
		sf::Packet pack;
		int numCartas = Manos[i].size();
		int numCartasCopia = s.copiasManos[i].size();
		std::cout << "Cartas en esta mano: " << numCartas << "   &  Cartas en la copia del servidor:" << numCartasCopia << std::endl;
		pack << HEAD::MANO << numCartas;
		for (auto it = Manos[i].begin(); it != Manos[i].end(); it++)
		{
			pack << it->cartatipo << it->nombre;
		}
		enviar(s.JugadoresSala[i], pack);
	}


	/*int cartasXmano = 42 / s.numPlayers;
	int cartasRestantes = 42;
	for (int i = 0; i < s.numPlayers; i++)
	{
		sf::Packet mano;
		mano << HEAD::MANO << cartasXmano;
		for (int i = 0; i < cartasXmano; i++)
		{
			int carta = rand() % cartasRestantes;
			mano << baraja[carta].cartatipo << baraja[carta].nombre;
			baraja.erase(baraja.begin() + carta);
			cartasRestantes--;
		}
		enviar(s.JugadoresSala[i], mano);
	}*/
}

void unirseaSala(sf::Packet _pack, sf::TcpSocket* _sock)
{
	std::string nombre;
	std::string contraseña;
	_pack >> nombre;
	_pack >> contraseña;
	std::cout << "Nombre recibido:  " << nombre << std::endl;
	std::cout << "Nombre Sala existente:  " << Salas[nombre].nombre << std::endl;
	if (nombre.compare(Salas[nombre].nombre) == 0)
	{
		if (contraseña.compare(Salas[nombre].password) == 0)
		{
			Salas[nombre].lastID++;
			Salas[nombre].playersConected++;
			std::pair<int, sf::TcpSocket*> newPlayer;
			newPlayer.first = Salas[nombre].lastID;
			newPlayer.second = _sock;
			Salas[nombre].JugadoresSala.insert(newPlayer);
			std::cout << "Se ha añadido un jugador nuevo a la sala " << Salas[nombre].nombre;
			_pack.clear();
			_pack << HEAD::NEWPLAYER
				<< newPlayer.first
				<< newPlayer.second->getRemoteAddress().toString()    //El orden es ID,IP,PUERTO
				<< newPlayer.second->getRemotePort();
			for (size_t i = 0; i < Salas[nombre].lastID; i++)
			{
				enviar(Salas[nombre].JugadoresSala[i], _pack);
			}

			_pack.clear();
		}
		else
		{
			std::cout << "Contraseña recibido:  " << contraseña << std::endl;
			std::cout << "Contraseña Sala existente:  " << Salas[nombre].password << std::endl;
			_pack.clear();
			_pack << HEAD::ASKNAME;
			enviar(_sock, _pack);
		}
		if (Salas[nombre].playersConected == Salas[nombre].maxPlayers) {
			_pack.clear();
			repartirCartas(Salas[nombre]);
			//_pack << HEAD::STARTGAME;
		}

	}
	else
	{
		_pack.clear();
		_pack << HEAD::ASKNAME;
		enviar(_sock, _pack);
	}


}

void enviarSalas(sf::TcpSocket* _sock)
{
	sf::Packet pack;
	pack << HEAD::LISTASALAS << numSalas;
	for (auto it = Salas.begin(); it != Salas.end(); it++)
	{
		pack << it->first;
	}
	enviar(_sock, pack);
}

void enviarSalasFiltro(sf::Packet _pack, sf::TcpSocket* _sock)
{
	sf::Packet pack;
	int num;
	_pack >> num;
	int salasFiltradas = 0;
	for (auto it = Salas.begin(); it != Salas.end(); it++)
	{
		if (it->second.maxPlayers == num)
			salasFiltradas++;
	}
	pack << HEAD::LISTASALAS << salasFiltradas;
	for (auto it = Salas.begin(); it != Salas.end(); it++)
	{
		/*std::cout << "numsalas" << Salas[it->first].numPlayers << std::endl;
		std::cout << num << "miU";*/
		if (it->second.maxPlayers == num) { //como hacer esto
			pack << it->first;
		}

	}
	enviar(_sock, pack);
}

void enviarSalasAbiertas(sf::Packet _pack, sf::TcpSocket* _sock)
{
	sf::Packet pack;
	int num;
	_pack >> num;
	int salasFiltradas = 0;
	for (auto it = Salas.begin(); it != Salas.end(); it++)
	{
		if (it->second.password.size() == 0)
			salasFiltradas++;
	}
	pack << HEAD::LISTASALAS << salasFiltradas;
	for (auto it = Salas.begin(); it != Salas.end(); it++)
	{
		/*std::cout << "numsalas" << Salas[it->first].numPlayers << std::endl;
		std::cout << num << "miU";*/
		if (it->second.password.size() == 0) { //como hacer esto
			pack << it->first;
		}

	}
	enviar(_sock, pack);
}
void startGame(std::string nombreSala)
{
	Salas[nombreSala];
	sf::Packet pack;
	for (int i = 0; i < Salas[nombreSala].maxPlayers; i++)
	{
		sf::Packet pack;
		pack << HEAD::STARTGAME << playerTurn << Salas[nombreSala].maxPlayers << i << nombreSala;
		enviar(Salas[nombreSala].JugadoresSala[i], pack);
		pack.clear();
	}
	pack << HEAD::YOURTURN;
	enviar(Salas[nombreSala].JugadoresSala[playerTurn], pack);
	pack.clear();
}
void nextTurn(sf::Packet _pack)
{
	std::string nombreSala;
	_pack >> nombreSala;
	playerTurn++;
	if (playerTurn >= Salas[nombreSala].maxPlayers)
	{
		playerTurn = 0;
	}

	sf::Packet pack;
	for (int i = 0; i < Salas[nombreSala].maxPlayers; i++)
	{
		if (i != playerTurn)
		{
			pack << CAMBIODETURNO << playerTurn;
			enviar(Salas[nombreSala].JugadoresSala[playerTurn], pack);
			pack.clear();
		}
		else
		{
			pack << HEAD::YOURTURN;
			enviar(Salas[nombreSala].JugadoresSala[playerTurn], pack);
			pack.clear();

		}

	}

}

void checkPlayerReady(sf::TcpSocket* _sock)
{
	for (auto it = Salas.begin(); it != Salas.end(); it++)
	{
		for (int i = 0; i < it->second.maxPlayers; i++)
		{
			if (it->second.JugadoresSala[i] == _sock)
			{
				it->second.playersReady++;
			}
		}
		if (it->second.playersReady >= it->second.maxPlayers)
		{
			startGame(it->first);
			break;
		}

	}


}

void enviarPregunta(sf::Packet _pack, sf::TcpSocket* _sock)
{
	//pack << miembro << t << jugadorPreguntado;
	sf::Packet pack;
	int tribu;
	int jugadorPreguntado;
	int jugadorQuePregunta;
	std::string nombreSala;
	std::string miembro;
	_pack >> nombreSala;
	_pack >> miembro;
	_pack >> tribu;
	_pack >> jugadorPreguntado;
	_pack >> jugadorQuePregunta;

	for (size_t i = 0; i < Salas[nombreSala].maxPlayers; i++)
	{
		if (Salas[nombreSala].JugadoresSala[i] != _sock)
		{
			if (i == jugadorPreguntado)
			{
				pack.clear();
				pack << PREGUNTA << miembro << tribu << jugadorQuePregunta;
				enviar(Salas[nombreSala].JugadoresSala[i], pack);
				pack.clear();
			}
			else
			{
				pack.clear();
				std::string info;
				info = "El jugador " + std::to_string(jugadorQuePregunta) + " pregunta al jugador " + std::to_string(jugadorPreguntado) + " si tiene la carta " + miembro + " " + std::to_string(tribu);
				pack << INFO << info;
				enviar(Salas[nombreSala].JugadoresSala[i], pack);
				pack.clear();
			}
		}
	}
}
void eliminarTribu(Sala &s, int id, int tribu)
{
	/*for (auto it = s.copiasManos[id].begin(); it != s.copiasManos[id].end(); it++)
	{
		std::cout << it->nombre << " " << it->cartatipo << std::endl;

	}*/
	int numCartas = s.copiasManos[id].size();
	for (int i = 0; i < numCartas; i++)
	{
		if (s.copiasManos[id][i].cartatipo == (tipo)tribu)
		{
			s.copiasManos[id].erase(s.copiasManos[id].begin() + i);
			numCartas--;
			i--;
		}

	}
	for (auto it = s.copiasManos[id].begin(); it != s.copiasManos[id].end(); it++)
	{
		std::cout << it->nombre << " " << it->cartatipo << std::endl;
	}

}
void sumarFamilia(sf::Packet _pack)
{
	std::cout << "entro funcion suma" << std::endl;
	//	respuesta << HEAD::DARCARTA << miembro << tribu << jugadorQuePregunta;

	std::string miembro;
	int tribu;
	int jugadorQuePregunta;
	std::string nombreSala;
	std::string info;
	_pack >> tribu >> jugadorQuePregunta >> nombreSala;
	info = "El jugador " + std::to_string(jugadorQuePregunta) + " ha completado la tribu de los " + ConversorString((tipo)tribu);
	sf::Packet carta;
	carta << miembro << tribu;

	eliminarTribu(Salas[nombreSala], jugadorQuePregunta, tribu);

	Salas[nombreSala].puntos[jugadorQuePregunta]++;
	//std::cout << Salas[nombreSala].puntos[jugadorQuePregunta] << std::endl;
	enviar(Salas[nombreSala].JugadoresSala[jugadorQuePregunta], carta);
	carta.clear();
	for (int i = 0; i < Salas[nombreSala].playersConected; i++)
	{
		if (i != jugadorQuePregunta)
		{
			carta << HEAD::INFO << info;
			enviar(Salas[nombreSala].JugadoresSala[i], carta);
			carta.clear();
		}
	}
	int sumPuntos = 0;
	for (int i = 0; i < Salas[nombreSala].playersConected; i++)
	{
		sumPuntos = sumPuntos + Salas[nombreSala].puntos[i];
	}
	if (sumPuntos >= 6)
	{
		enviarFinalPartida(Salas[nombreSala]);
	}
}
void cambiarCartasDeMano(Sala &s, std::string nombre, int tribu, int idJugadorQueRecibeCarta)
{
	for (int i = 0; i < s.playersConected; i++)
	{
		for (int j = 0; j < s.copiasManos[i].size(); j++)
		{
			if (s.copiasManos[i][j].cartatipo == (tipo)tribu && s.copiasManos[i][j].nombre == nombre)
			{
				s.copiasManos[i].erase(s.copiasManos[i].begin() + j);
			}
		}
	}
	carta c((tipo)tribu, nombre);
	s.copiasManos[idJugadorQueRecibeCarta].push_back(c);

}
void enviarCarta(sf::Packet _pack)
{
	//	respuesta << HEAD::DARCARTA << miembro << tribu << jugadorQuePregunta;
	std::string miembro;
	int tribu;
	int jugadorQuePregunta;
	std::string nombreSala;
	std::string info;
	_pack >> miembro >> tribu >> jugadorQuePregunta >> nombreSala;
	info = "El jugador " + std::to_string(jugadorQuePregunta) + " ha recibido una carta";
	sf::Packet carta;
	carta << HEAD::DARCARTA << miembro << tribu;
	enviar(Salas[nombreSala].JugadoresSala[jugadorQuePregunta], carta);
	carta.clear();
	cambiarCartasDeMano(Salas[nombreSala], miembro, tribu, jugadorQuePregunta);
	for (size_t i = 0; i < Salas[nombreSala].maxPlayers; i++)
	{
		if (i != jugadorQuePregunta)
		{
			carta << HEAD::INFO << info;
			enviar(Salas[nombreSala].JugadoresSala[i], carta);
			carta.clear();
		}
	}

	//carta << HEAD::COMPROBARFAMILIAS;
}
void actualizarID(int jugadorEliminaldoID, Sala &s)
{
	for (auto it = s.JugadoresSala.begin(); it != s.JugadoresSala.end(); it++)
	{
		if (jugadorEliminaldoID < it->first)
		{
			sf::TcpSocket* handler;
			handler = it->second;
			int nuevaID = it->first;
			s.JugadoresSala.erase(it);
			nuevaID--;

			s.JugadoresSala.insert(std::pair<int, sf::TcpSocket*>(nuevaID, handler));
			sf::Packet pack;
			pack << HEAD::NEWID << nuevaID;
			enviar(s.JugadoresSala[nuevaID], pack);
			pack.clear();
		}
	}

}
void enviarNegativa(sf::Packet _pack)
{
	int jugadorDestino;
	std::string nombreSala;
	std::string info;
	_pack >> jugadorDestino >> nombreSala;
	sf::Packet pack;
	info = "El jugador " + std::to_string(jugadorDestino) + " no ha recibido ninguna carta";
	pack << HEAD::NOCARTA;
	enviar(Salas[nombreSala].JugadoresSala[jugadorDestino], pack);
	pack.clear();

	for (size_t i = 0; i < Salas[nombreSala].maxPlayers; i++)
	{
		if (i != jugadorDestino)
		{
			pack << HEAD::INFO << info;
			enviar(Salas[nombreSala].JugadoresSala[i], pack);
			pack.clear();
		}
	}

}
void enviarDesconexion(sf::TcpSocket* sock)
{
	std::string nombreSala;
	int idJugador;
	bool esta_en_sala = false;

	for (auto it = Salas.begin(); it != Salas.end(); it++)
	{
		for (int i = 0; i < it->second.maxPlayers; i++)
		{
			if (it->second.JugadoresSala[i] == sock)
			{
				nombreSala = it->first;
				idJugador = i;
				esta_en_sala = true;
				break;
			}
		}
	}
	if (esta_en_sala)
	{
		sf::Packet pack;
		Salas[nombreSala].JugadoresSala.erase(idJugador);
		actualizarID(idJugador, Salas[nombreSala]);
		Salas[nombreSala].playersConected--;
		Salas[nombreSala].lastID--;
		std::vector<std::vector<carta>>Manos;
		for (int i = 0; i < Salas[nombreSala].playersConected; i++) //Generamos las manos necesarias
		{
			std::vector<carta>mano;
			Manos.push_back(mano);
		}
		int cartasRestantes = Salas[nombreSala].copiasManos[idJugador].size();
		while (cartasRestantes != 0)
		{
			for (int i = 0; i < Salas[nombreSala].playersConected && cartasRestantes > 0; i++)
			{
				//int azar = rand() % cartasRestantes;
				carta repartida(Salas[nombreSala].copiasManos[idJugador][0].cartatipo, Salas[nombreSala].copiasManos[idJugador][0].nombre);
				Manos[i].push_back(repartida);
				Salas[nombreSala].copiasManos[i].push_back(repartida);
				Salas[nombreSala].copiasManos[idJugador].erase(Salas[nombreSala].copiasManos[idJugador].begin() + 0);
				cartasRestantes--;
			}
		}


		for (int i = 0; i < Salas[nombreSala].playersConected; i++)
		{
			// idJugador << numCartas << nombre << tipo ...
			int numCartas = Manos[i].size();
			pack << HEAD::JUGADORDESCONECTADO << idJugador << numCartas;
			for (auto it = Manos[i].begin(); it != Manos[i].end(); it++)
			{
				pack << it->cartatipo << it->nombre;
			}
			enviar(Salas[nombreSala].JugadoresSala[i], pack);
			pack.clear();

		}
	}
	if (Salas[nombreSala].playersConected <= 2)
	{
		enviarFinalPartida(Salas[nombreSala]);
	}
	if (playerTurn==idJugador) {
		playerTurn++;
		if (playerTurn >= Salas[nombreSala].maxPlayers)
		{
			playerTurn = 0;
		}

		sf::Packet pack;
		for (int i = 0; i < Salas[nombreSala].maxPlayers; i++)
		{
			if (i != playerTurn)
			{
				pack << CAMBIODETURNO << playerTurn;
				enviar(Salas[nombreSala].JugadoresSala[playerTurn], pack);
				pack.clear();
			}
			else
			{
				pack << HEAD::YOURTURN;
				enviar(Salas[nombreSala].JugadoresSala[playerTurn], pack);
				pack.clear();

			}

		}
	}
}

void recibir(sf::TcpSocket* sock)
{
	bool conectado = true;
	while (conectado)
	{
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
				break;

			case HEAD::DESCONEXION:
				conectado = false;
				partida = false;
				break;
			case HEAD::LISTA:

				updatePeers(pack);
				break;
			case HEAD::CHAT:
				pack >> tmp;
				std::cout << tmp << std::endl;
				break;
			case HEAD::PEDIRSALAS:
				enviarSalas(sock);
				break;
			case HEAD::PEDIRFILTROSALAS:
				enviarSalasFiltro(pack, sock);
				break;
			case HEAD::PEDIRSALASABIERTAS:
				enviarSalasAbiertas(pack, sock);
				break;
			case HEAD::UNIRME:
				std::cout << "Un jugador esta intentando unirse a una partida\n";
				unirseaSala(pack, sock);
				break;
			case HEAD::CREARSALA:
				crearSala(pack, sock);
				break;
			case HEAD::READY:
				checkPlayerReady(sock);
				break;
			case HEAD::PREGUNTA:
				enviarPregunta(pack, sock);
				break;
			case HEAD::DARCARTA:
				enviarCarta(pack);
				break;
			case HEAD::NEXTTURN:
				nextTurn(pack);
				break;
			case HEAD::NOCARTA:
				enviarNegativa(pack);
				break;
			case HEAD::COMPROBARFAMILIAS:
				sumarFamilia(pack);
				break;
			default:
				break;
			}
			//std::cout << cabecera << std::endl;
		}
		else if (status == sf::Socket::Disconnected)
		{
			std::cout << "Desconectado\n";
			enviarDesconexion(sock);
			conectado = false;
			break;
		}
		pack.clear();
	}

}
void Recepcion() {

	while (partida)
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
bool Servidor() {
	sf::TcpSocket *_socket = new sf::TcpSocket();
	//Listener escuchando por un puerto
	sf::Socket::Status status;
	//Print de la ip:puerto por el que se escucha
	sf::TcpListener listener;
	//Accept para esperar socket nuevo
	status = listener.listen(55555);
	listener.setBlocking(true);
	if (status != sf::Socket::Done) {
		std::cout << "Error en el listener....cerrando el programa";
		return false;
	}
	else {
		sf::IpAddress ip = sf::IpAddress::LocalHost;
		//std::cout << ip.getLocalAddress() << ";" << std::to_string(55555) << std::endl;

		//Accept para esperar socket nuevo
		sf::Socket::Status statusL;
		std::cout << "Esperando nueva conexion \n";
		statusL = listener.accept(*_socket);

		if (statusL != sf::Socket::Done) {
			std::cout << "Error en el listener....cerrando el programa\n";
			return false;
		}
		else {
			std::cout << "Se ha conectado un socket\n";
			clients.push_back(_socket);
			std::thread recibir(recibir, _socket);
			recibir.detach();

			return true;
		}
		//std::cout << "me da igual todo\n";
		listener.close();
	}
}

void BSS()
{
	while (partida)
	{
		Servidor();
	}
}

void endGame()
{
	std::string input;

	std::cin.ignore();
	std::getline(std::cin, input);
}

int main() {

	crearbaraja();
	sf::TcpSocket* sock = new sf::TcpSocket();
	_BSS = new sf::TcpSocket();
	//std::cout << clients.size() << std::endl;
		//host = new BSS;
	okConexion = Servidor();
	std::thread threadconexiones(BSS);
	threadconexiones.detach();
	//okConexion = true;




	if (okConexion)
	{
		Recepcion();

	}
	sock->disconnect();
	delete sock;
	endGame();
	return 0;
}
