// ============================================================================
//  MOTO XYZ 250 - APOCALIPSIS
//  Juego 2D de motos con rampas, monstruos y un arma.
// ============================================================================
//
//  Este archivo esta escrito para que lo pueda leer alguien que apenas
//  empieza a programar. Casi cada linea importante tiene su explicacion.
//
//  NOVEDADES DE ESTA VERSION:
//    * CHOQUE REALISTA: cuando MUERES (te matan o te caes) hay un instante de
//      camara lenta. El piloto sale disparado con la velocidad que llevabas
//      (inercia), da vueltas, rebota, se arrastra por el suelo con el cuerpo
//      flojo (ragdoll) y dice "AH GONORREA!". La moto se vuelca, rueda y se
//      arrastra sacando chispas. La camara sigue al piloto.
//    * MESSI ahora ATACA: se para a cierta distancia y te PATEA BALONES DE
//      FUTBOL. Los balones vuelan en arco, rebotan en el suelo y te hacen dano
//      si te tocan. Puedes DESTRUIRLOS A BALAZOS (+5 puntos). Messi TAMBIEN
//      se puede matar a balazos (3 disparos) o atropellandolo.
//    * El monstruo GRANDE (antes "bruto") ahora es "ESTRELLA POP": un cantante
//      pop ficticio con chamarra brillante y microfono.
//
//  ----------------------------------------------------------------------------
//  COMO COMPILAR
//  ----------------------------------------------------------------------------
//  A) SIN ROS (lo mas facil, solo necesitas SDL2):
//
//       sudo apt install g++ libsdl2-dev
//       g++ -std=c++17 -O2 -DNO_ROS moto_game.cpp -o moto_game $(sdl2-config --cflags --libs)
//       ./moto_game
//
//  B) CON ROS 2 (como tu bike_game.cpp original):
//     Usa tu mismo CMakeLists.txt, solo cambia el nombre del archivo fuente
//     (o guarda este archivo con el nombre bike_game.cpp). No pongas -DNO_ROS.
//
//  Se necesita SDL2 version 2.0.18 o mas nueva (para dibujar poligonos con
//  degradados). Ubuntu 22.04 y 24.04 ya la traen.
//
//  ----------------------------------------------------------------------------
//  CONTROLES
//  ----------------------------------------------------------------------------
//    W / Flecha arriba     -> acelerar
//    S / Flecha abajo      -> frenar
//    D / Flecha derecha    -> inclinarte hacia adelante (en el aire: giro adelante)
//    A / Flecha izquierda  -> inclinarte hacia atras (caballito / giro atras)
//    MOUSE                 -> apuntar el arma (hay una mira en pantalla)
//    Click izquierdo o ESPACIO -> disparar
//    R                     -> reiniciar el juego desde cero
//    F12                   -> guardar captura de pantalla (captura.bmp)
//    ESC                   -> salir
//
//  CONTROL DE XBOX 360 (o cualquier control que SDL reconozca):
//    Gatillo derecho (RT)  -> acelerar (mientras mas lo aprietas, mas acelera)
//    Boton A               -> disparar
//    Boton B               -> frenar
//    Joystick DERECHO      -> apuntar el arma (hacia donde empujes el stick)
//    Joystick IZQUIERDO    -> inclinar la moto, eje X (derecha = adelante, izquierda = atras)
//    Boton START           -> reiniciar el juego
//    (El teclado y el mouse siguen funcionando al mismo tiempo.)
//
//  ----------------------------------------------------------------------------
//  COMO SE JUEGA
//  ----------------------------------------------------------------------------
//   * Llega a la META (arco de cuadros blancos y negros) sin caerte.
//   * Si la cabeza o la cadera del piloto tocan el suelo, te caes.
//   * Hay 3 tipos de monstruos: PERSONAJES (caminan: Pablo Escobar, Gengis Kan,
//     Hitler y Messi), MURCIELAGO (vuela) y ESTRELLA POP (grande).
//   * MESSI te lanza balones de futbol. Esquivalos, destruyelos disparandoles,
//     o mata a Messi a balazos (3 disparos) para que deje de patear.
//   * Si vas muy rapido puedes ATROPELLAR a un personaje sin recibir dano
//     (Messi es inofensivo si solo lo tocas despacio).
//   * Recoge cajas de balas (amarillas) y botiquines (cruz roja).
//   * Cada cierto tramo hay un CHECKPOINT (bandera): si mueres, reapareces ahi.
//
//  ----------------------------------------------------------------------------
//  TOPICS DE ROS 2 (solo si compilas con ROS)
//  ----------------------------------------------------------------------------
//    publica   /score         (std_msgs/Int32)          puntos
//    publica   /moto/health   (std_msgs/Int32)          vida 0..100
//    publica   /bike/pose     (geometry_msgs/PoseStamped)  posicion de la moto
//    suscribe  /bike/cmd_vel  (geometry_msgs/Twist)     linear.x = acelerar, angular.z = inclinar
//    suscribe  /moto/fire     (std_msgs/Bool)           true = disparar
//
//  ----------------------------------------------------------------------------
//  MAPA DEL CODIGO (en este orden)
//  ----------------------------------------------------------------------------
//    1. Includes y configuracion (constantes que puedes cambiar)
//    2. Matematicas basicas (Vec2, colores, numeros "aleatorios" repetibles)
//    3. Fuente de letras hecha a mano (para dibujar texto sin librerias)
//    4. Gfx: nuestro "motor grafico" (dibuja poligonos, circulos, lineas...)
//    5. Track: el terreno, las rampas y la decoracion
//    6. Moto: la fisica de la moto XYZ 250
//    7. Piloto, monstruos, balas, balones, pickups y particulas (estructuras de datos)
//    8. Game: la logica del juego + todo el dibujado
//    9. (opcional) Nodo de ROS 2 y funcion main
//
//  IDEA CLAVE PARA NOVATOS:
//    Un videojuego es un bucle que se repite ~60 veces por segundo:
//        leer teclado -> actualizar el mundo (fisica, enemigos) -> dibujar
//    En este codigo ese bucle es la funcion Game::tick().
//
//    Coordenadas: x crece hacia la DERECHA, y crece hacia ABAJO (como en casi
//    todos los motores 2D). Por eso "subir" significa restar a y.
// ============================================================================

// ---------------------------------------------------------------------------
// 1. INCLUDES Y CONFIGURACION
// ---------------------------------------------------------------------------


#include <SDL2/SDL.h>

#include <algorithm>   // std::min, std::max, std::clamp, std::remove_if
#include <cctype>      // std::toupper
#include <chrono>      // para medir tiempo (comandos de ROS)
#include <cmath>       // sin, cos, sqrt, atan2...
#include <cstdint>     // tipos como uint8_t
#include <cstdio>      // snprintf
#include <memory>      // std::make_shared
#include <random>      // numeros aleatorios
#include <stdexcept>   // std::runtime_error
#include <string>
#include <vector>

// Si NO compilas con -DNO_ROS, se incluye ROS 2.
#ifndef NO_ROS
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_msgs/msg/int32.hpp"
#endif

#if !SDL_VERSION_ATLEAST(2, 0, 18)
#error "Necesitas SDL2 version 2.0.18 o superior (instala libsdl2-dev mas nuevo)."
#endif

namespace
{

// ----- Ventana -----
constexpr int WIN_W = 1100;               // ancho de la ventana en pixeles
constexpr int WIN_H = 620;                // alto de la ventana en pixeles
constexpr double DT = 1.0 / 60.0;         // cada "paso" de juego dura 1/60 s
constexpr double PI = 3.14159265358979323846;

// ----- Pista -----
constexpr double TRACK_LEN = 15000.0;     // largo total de la pista (pixeles)
constexpr double STEP_X = 8.0;            // el terreno se guarda cada 8 px
constexpr double FINISH_X = TRACK_LEN - 400.0;   // donde esta la meta
constexpr double CHECKPOINT_GAP = 3800.0; // distancia entre checkpoints

// ----- Moto XYZ 250 (fisica) -----
constexpr double WHEEL_R = 25.0;          // radio de las ruedas
constexpr double WHEEL_BASE = 100.0;       // distancia entre eje trasero y delantero
constexpr double GRAVITY = 1300.0;        // que tan fuerte cae (mas = mas pesado)
constexpr double DRIVE = 3000.0;          // fuerza del motor
constexpr double MAX_DRIVE_SPEED = 850.0; // velocidad maxima que el motor puede dar
constexpr double LEAN = 2200.0;           // fuerza para inclinar la moto
constexpr double DAMPING = 0.9995;        // rozamiento del aire (1.0 = ninguno)
constexpr int SUBSTEPS = 4;               // cada paso se divide en 4 (mas precision)
constexpr int ITERATIONS = 8;             // repeticiones al resolver choques

// ----- Jugador y arma -----
constexpr double MAX_HEALTH = 100.0;      // vida maxima
long constexpr int START_AMMO = 20000;            // balas al empezar
constexpr int MAX_AMMO = 20000;              // maximo de balas
constexpr double FIRE_DELAY = 0.001;       // segundos entre disparos
constexpr double BULLET_SPEED = 1600.0;   // velocidad de la bala (px/s)

// ----- Control (gamepad) -----
constexpr double PAD_DEADZONE = 0.25;     // los sticks nunca vuelven EXACTO a 0: ignoramos lo menor a esto
constexpr Sint16 PAD_TRIGGER_ON = 8000;   // el gatillo va de 0 a 32767; mas que esto = "apretado"

// ----- Dificultad (cambialos para hacerlo mas facil o dificil) -----
constexpr double ACTIVATE_DIST = 1050.0;  // a que distancia "despiertan" los monstruos
constexpr double DAMAGE_ZOMBI = 12.0;     // dano que hace un personaje (Escobar, Gengis Kan, Hitler)
constexpr double DAMAGE_BAT = 8.0;        // dano del murcielago
constexpr double DAMAGE_BRUTO = 22.0;     // dano de la Estrella Pop (el monstruo grande)
constexpr double RUN_OVER_SPEED = 330.0;  // velocidad minima para atropellar personajes

// ----- Balones de Messi -----
constexpr double DAMAGE_BALL = 10.0;      // dano que te hace un balon
constexpr double BALL_R = 11.0;           // radio del balon
constexpr double BALL_GRAVITY = 900.0;    // que tan rapido cae el balon (mas bajo que la moto: es ligero)
constexpr double MESSI_KICK_RANGE = 750.0;   // Messi solo patea si estas a menos de esta distancia
constexpr double MESSI_STAND_DIST = 320.0;   // Messi se acerca hasta esta distancia y se queda parado
constexpr double MESSI_KICK_DELAY = 1.6;     // segundos minimos entre patadas (se le suma un poco al azar)

constexpr size_t MAX_FX = 1600;           // maximo de particulas a la vez

// ---------------------------------------------------------------------------
// 2. MATEMATICAS BASICAS
// ---------------------------------------------------------------------------

// Vec2 = un punto o una flecha en 2D (x, y). Lo usamos para posiciones y velocidades.
struct Vec2
{
  double x{0.0}, y{0.0};
};
inline Vec2 operator+(Vec2 a, Vec2 b) {return {a.x + b.x, a.y + b.y};}
inline Vec2 operator-(Vec2 a, Vec2 b) {return {a.x - b.x, a.y - b.y};}
inline Vec2 operator*(Vec2 a, double s) {return {a.x * s, a.y * s};}
inline Vec2 operator/(Vec2 a, double s) {return {a.x / s, a.y / s};}
inline Vec2 & operator+=(Vec2 & a, Vec2 b) {a.x += b.x; a.y += b.y; return a;}
inline Vec2 & operator-=(Vec2 & a, Vec2 b) {a.x -= b.x; a.y -= b.y; return a;}
inline double dot(Vec2 a, Vec2 b) {return a.x * b.x + a.y * b.y;}   // producto punto
inline double len(Vec2 a) {return std::sqrt(dot(a, a));}            // largo de la flecha
inline Vec2 normalized(Vec2 a)                                      // flecha de largo 1
{
  const double l = len(a);
  return l > 1e-9 ? a / l : Vec2{1.0, 0.0};
}
inline Vec2 rotated(Vec2 v, double a)                               // gira una flecha 'a' radianes
{
  const double c = std::cos(a), s = std::sin(a);
  return {v.x * c - v.y * s, v.x * s + v.y * c};
}
inline Vec2 lerp(Vec2 a, Vec2 b, double t) {return a + (b - a) * t;}
inline double lerp(double a, double b, double t) {return a + (b - a) * t;}

// Distancia de un punto 'p' al segmento (recta corta) que va de 'a' a 'b'.
// La usamos para saber si una bala (que se mueve rapido) toco a un monstruo.
double distPointSegment(Vec2 p, Vec2 a, Vec2 b)
{
  const Vec2 ab = b - a;
  const double l2 = dot(ab, ab);
  const double t = l2 > 1e-9 ? std::clamp(dot(p - a, ab) / l2, 0.0, 1.0) : 0.0;
  return len(p - (a + ab * t));
}

// "Aleatorio repetible": siempre da el mismo numero (entre 0 y 1) para el mismo n.
// Sirve para colocar edificios y arboles "al azar" pero igual en cada partida.
inline double hash01(int n)
{
  uint32_t x = static_cast<uint32_t>(n) * 747796405u + 2891336453u;
  x = ((x >> ((x >> 28u) + 4u)) ^ x) * 277803737u;
  x = (x >> 22u) ^ x;
  return x / 4294967295.0;
}

// Hace que v "de la vuelta" dentro de [0, m). Util para fondos que se repiten.
inline double wrapTo(double v, double m) {return v - m * std::floor(v / m);}

// Color = el SDL_Color de siempre (r, g, b, a). 'a' es la transparencia (255 = opaco).
using Color = SDL_Color;
inline Color rgb(int r, int g, int b, int a = 255)
{
  return Color{static_cast<Uint8>(r), static_cast<Uint8>(g), static_cast<Uint8>(b),
    static_cast<Uint8>(a)};
}
// Mezcla dos colores: t=0 da 'a', t=1 da 'b'.
inline Color mix(Color a, Color b, double t)
{
  t = std::clamp(t, 0.0, 1.0);
  return rgb(
    static_cast<int>(lerp(a.r, b.r, t)), static_cast<int>(lerp(a.g, b.g, t)),
    static_cast<int>(lerp(a.b, b.b, t)), static_cast<int>(lerp(a.a, b.a, t)));
}
// Devuelve el mismo color pero con la transparencia multiplicada por 'k' (0..1).
inline Color fade(Color c, double k)
{
  c.a = static_cast<Uint8>(std::clamp(c.a * k, 0.0, 255.0));
  return c;
}

// ---------------------------------------------------------------------------
// 3. FUENTE DE LETRAS HECHA A MANO
// ---------------------------------------------------------------------------
// Cada letra es una cuadricula de 5 columnas x 7 filas. Cada fila es un numero
// binario: un 1 = pixel encendido, un 0 = apagado. Ejemplo, la letra "A":
//      01110      .XXX.
//      10001      X...X
//      10001      X...X
//      11111      XXXXX
//      10001      X...X
//      10001      X...X
//      10001      X...X
// Solo hay MAYUSCULAS, numeros y unos pocos simbolos (sin acentos).

// El caracter '#' se dibuja como el signo de exclamacion de apertura (el de arriba hacia abajo).
const std::string kFontChars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ:/-!.+ #";

constexpr uint8_t kFont[44][7] = {
  {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110},  // 0
  {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},  // 1
  {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111},  // 2
  {0b11110, 0b00001, 0b00001, 0b01110, 0b00001, 0b00001, 0b11110},  // 3
  {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010},  // 4
  {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110},  // 5
  {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110},  // 6
  {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000},  // 7
  {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110},  // 8
  {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100},  // 9
  {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001},  // A
  {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110},  // B
  {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110},  // C
  {0b11100, 0b10010, 0b10001, 0b10001, 0b10001, 0b10010, 0b11100},  // D
  {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111},  // E
  {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000},  // F
  {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111},  // G
  {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001},  // H
  {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},  // I
  {0b00111, 0b00010, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100},  // J
  {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001},  // K
  {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111},  // L
  {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001},  // M
  {0b10001, 0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001},  // N
  {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},  // O
  {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000},  // P
  {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101},  // Q
  {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001},  // R
  {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110},  // S
  {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100},  // T
  {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},  // U
  {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100},  // V
  {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010},  // W
  {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001},  // X
  {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100},  // Y
  {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111},  // Z
  {0b00000, 0b00100, 0b00100, 0b00000, 0b00100, 0b00100, 0b00000},  // :
  {0b00001, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b10000},  // /
  {0b00000, 0b00000, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000},  // -
  {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000, 0b00100},  // !
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b01100, 0b01100},  // .
  {0b00000, 0b00100, 0b00100, 0b11111, 0b00100, 0b00100, 0b00000},  // +
  {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000},  // (espacio)
  {0b00100, 0b00000, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100},  // # (se dibuja como el signo de apertura)
};

// Devuelve la cuadricula de una letra, o nullptr si no la tenemos.
const uint8_t * glyphFor(char ch)
{
  const char up = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
  const size_t pos = kFontChars.find(up);
  return pos == std::string::npos ? nullptr : kFont[pos];
}

// ---------------------------------------------------------------------------
// 4. Gfx: NUESTRO MOTOR GRAFICO
// ---------------------------------------------------------------------------
// SDL por si solo solo dibuja lineas y rectangulos. Para que el juego se vea
// bien usamos SDL_RenderGeometry, que dibuja TRIANGULOS con colores distintos
// en cada esquina (esto da degradados y transparencias). Con triangulos se
// puede armar cualquier cosa:
//    * poligono relleno  -> varios triangulos saliendo del primer punto
//    * circulo           -> muchos triangulos como gajos de pizza
//    * linea gruesa      -> un rectangulo girado
//
// Esta clase tambien guarda la "vista" (la camara): todo lo que dibujamos se
// da en coordenadas del MUNDO y Gfx le resta la posicion de la camara para
// saber donde cae en la pantalla.

class Gfx
{
public:
  explicit Gfx(SDL_Renderer * ren) : ren_(ren) {}

  // Mueve la camara. Con (0,0) dibujas directo en coordenadas de pantalla (HUD).
  void setView(Vec2 v) {view_ = v;}
  Vec2 view() const {return view_;}

  // Convierte un punto del mundo a un punto de la pantalla.
  SDL_FPoint toScreen(Vec2 w) const
  {
    return SDL_FPoint{static_cast<float>(w.x - view_.x), static_cast<float>(w.y - view_.y)};
  }

  // Poligono relleno. IMPORTANTE: debe ser "convexo" (sin entradas hacia adentro)
  // para que se vea bien. Los puntos se dan en orden alrededor de la figura.
  void polygon(const std::vector<Vec2> & pts, Color c)
  {
    if (pts.size() < 3) {return;}
    verts_.clear();
    idx_.clear();
    for (const Vec2 & p : pts) {verts_.push_back(makeVertex(toScreen(p), c));}
    for (int i = 1; i + 1 < static_cast<int>(pts.size()); ++i) {
      idx_.push_back(0);
      idx_.push_back(i);
      idx_.push_back(i + 1);
    }
    flush();
  }

  // Elipse rellena (si rx == ry es un circulo). 'seg' = cuantos gajos usa.
  void ellipse(Vec2 c, double rx, double ry, Color col, int seg = 24)
  {
    verts_.clear();
    idx_.clear();
    verts_.push_back(makeVertex(toScreen(c), col));   // vertice 0 = el centro
    for (int i = 0; i < seg; ++i) {
      const double a = 2.0 * PI * i / seg;
      verts_.push_back(makeVertex(toScreen({c.x + rx * std::cos(a), c.y + ry * std::sin(a)}), col));
    }
    for (int i = 0; i < seg; ++i) {
      idx_.push_back(0);
      idx_.push_back(1 + i);
      idx_.push_back(1 + (i + 1) % seg);
    }
    flush();
  }
  void circle(Vec2 c, double r, Color col, int seg = 24) {ellipse(c, r, r, col, seg);}

  // Resplandor: circulo cuyo centro tiene un color y el borde otro (normalmente
  // el mismo color pero transparente). Sirve para luces, fuego, explosiones.
  void glow(Vec2 c, double r, Color center, Color edge, int seg = 28)
  {
    verts_.clear();
    idx_.clear();
    verts_.push_back(makeVertex(toScreen(c), center));
    for (int i = 0; i < seg; ++i) {
      const double a = 2.0 * PI * i / seg;
      verts_.push_back(makeVertex(toScreen({c.x + r * std::cos(a), c.y + r * std::sin(a)}), edge));
    }
    for (int i = 0; i < seg; ++i) {
      idx_.push_back(0);
      idx_.push_back(1 + i);
      idx_.push_back(1 + (i + 1) % seg);
    }
    flush();
  }

  // Linea gruesa. Si round=true, las puntas son redondas (bueno para brazos y piernas).
  void line(Vec2 a, Vec2 b, double th, Color c, bool round = false)
  {
    const Vec2 d = b - a;
    const double l = len(d);
    if (l < 1e-6) {
      if (round) {circle(a, th * 0.5, c, 10);}
      return;
    }
    // 'n' es una flecha perpendicular a la linea, de largo = mitad del grosor.
    const Vec2 n{-d.y / l * th * 0.5, d.x / l * th * 0.5};
    polygon({a + n, b + n, b - n, a - n}, c);
    if (round) {
      circle(a, th * 0.5, c, 10);
      circle(b, th * 0.5, c, 10);
    }
  }

  // Anillo (circulo hueco).
  void ring(Vec2 c, double r, double th, Color col, int seg = 28)
  {
    verts_.clear();
    idx_.clear();
    for (int i = 0; i <= seg; ++i) {
      const double a = 2.0 * PI * i / seg;
      const Vec2 dir{std::cos(a), std::sin(a)};
      verts_.push_back(makeVertex(toScreen(c + dir * (r - th * 0.5)), col));
      verts_.push_back(makeVertex(toScreen(c + dir * (r + th * 0.5)), col));
    }
    for (int i = 0; i < seg; ++i) {
      const int a = 2 * i;
      idx_.insert(idx_.end(), {a, a + 1, a + 2, a + 1, a + 3, a + 2});
    }
    flush();
  }

  // Tira de cuadrilateros: une la lista 'top' (borde de arriba) con 'bot' (borde de
  // abajo). Con ella dibujamos el terreno entero en UNA sola llamada.
  void strip(const std::vector<Vec2> & top, const std::vector<Vec2> & bot, Color ct, Color cb)
  {
    const int n = static_cast<int>(std::min(top.size(), bot.size()));
    if (n < 2) {return;}
    verts_.clear();
    idx_.clear();
    for (int i = 0; i < n; ++i) {
      verts_.push_back(makeVertex(toScreen(top[i]), ct));   // vertice 2*i   = arriba
      verts_.push_back(makeVertex(toScreen(bot[i]), cb));   // vertice 2*i+1 = abajo
    }
    for (int i = 0; i < n - 1; ++i) {
      const int a = 2 * i;
      idx_.insert(idx_.end(), {a, a + 1, a + 2, a + 1, a + 3, a + 2});
    }
    flush();
  }

  // Rectangulo con un color distinto en cada esquina (arriba-izq, arriba-der,
  // abajo-der, abajo-izq). Sirve para degradados.
  void gradientRect(double x, double y, double w, double h, Color tl, Color tr, Color br, Color bl)
  {
    verts_.clear();
    idx_.clear();
    verts_.push_back(makeVertex(toScreen({x, y}), tl));
    verts_.push_back(makeVertex(toScreen({x + w, y}), tr));
    verts_.push_back(makeVertex(toScreen({x + w, y + h}), br));
    verts_.push_back(makeVertex(toScreen({x, y + h}), bl));
    idx_ = {0, 1, 2, 0, 2, 3};
    flush();
  }
  void rect(double x, double y, double w, double h, Color c) {gradientRect(x, y, w, h, c, c, c, c);}
  void vGradient(double x, double y, double w, double h, Color top, Color bottom)
  {
    gradientRect(x, y, w, h, top, top, bottom, bottom);
  }
  void hGradient(double x, double y, double w, double h, Color left, Color right)
  {
    gradientRect(x, y, w, h, left, right, right, left);
  }

  // ----- Texto -----
  // Dibuja texto con nuestra fuente de pixeles. Cada pixel encendido es un
  // cuadradito. 'right' y 'down' son las direcciones "derecha" y "abajo" del
  // texto: asi se puede dibujar texto girado (lo usamos en el tanque de la moto).
  void textOriented(const std::string & s, Vec2 origin, Vec2 right, Vec2 down, double scale, Color c)
  {
    verts_.clear();
    idx_.clear();
    double cx = 0.0;                                // cuantas columnas llevamos
    for (char ch : s) {
      const uint8_t * g = glyphFor(ch);
      if (g) {
        for (int r = 0; r < 7; ++r) {
          for (int k = 0; k < 5; ++k) {
            if (g[r] & (1 << (4 - k))) {
              const Vec2 p0 = origin + right * ((cx + k) * scale) + down * (r * scale);
              const Vec2 p1 = p0 + right * scale;
              const Vec2 p2 = p1 + down * scale;
              const Vec2 p3 = p0 + down * scale;
              const int base = static_cast<int>(verts_.size());
              verts_.push_back(makeVertex(toScreen(p0), c));
              verts_.push_back(makeVertex(toScreen(p1), c));
              verts_.push_back(makeVertex(toScreen(p2), c));
              verts_.push_back(makeVertex(toScreen(p3), c));
              idx_.insert(idx_.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
            }
          }
        }
      }
      cx += 6.0;                                     // 5 columnas + 1 de espacio
    }
    if (!verts_.empty()) {flush();}
  }

  // Texto normal (horizontal) con sombra para que se lea sobre cualquier fondo.
  void text(const std::string & s, double x, double y, double scale, Color c)
  {
    textOriented(s, {x + scale, y + scale}, {1, 0}, {0, 1}, scale, fade(rgb(0, 0, 0), c.a / 255.0 * 0.75));
    textOriented(s, {x, y}, {1, 0}, {0, 1}, scale, c);
  }
  static double textWidth(const std::string & s, double scale)
  {
    return s.empty() ? 0.0 : (s.size() * 6.0 - 1.0) * scale;
  }
  void textCentered(const std::string & s, double cx, double y, double scale, Color c)
  {
    text(s, cx - textWidth(s, scale) * 0.5, y, scale, c);
  }

private:
  static SDL_Vertex makeVertex(SDL_FPoint p, Color c)
  {
    SDL_Vertex v;
    v.position = p;
    v.color = c;
    v.tex_coord = SDL_FPoint{0.0f, 0.0f};
    return v;
  }
  // Manda los triangulos acumulados a la tarjeta grafica.
  void flush()
  {
    SDL_RenderGeometry(
      ren_, nullptr, verts_.data(), static_cast<int>(verts_.size()), idx_.data(),
      static_cast<int>(idx_.size()));
  }

  SDL_Renderer * ren_;
  Vec2 view_{0.0, 0.0};
  std::vector<SDL_Vertex> verts_;   // vertices (esquinas) del dibujo actual
  std::vector<int> idx_;            // cuales vertices forman cada triangulo
};

// ---------------------------------------------------------------------------
// 5. Track: EL TERRENO
// ---------------------------------------------------------------------------
// El terreno es una lista de alturas, una cada STEP_X pixeles. Entre dos
// puntos se une con una linea recta. La altura se calcula asi:
//     altura = terreno_base (colinas suaves) + suma de las rampas/obstaculos
// Los obstaculos estan en la tabla kFeatures: puedes editarla para cambiar el nivel.

enum class FeatureKind
{
  BUMP,     // loma suave (subes y bajas despacio)
  RAMP,     // rampa de salto: sube y se corta de golpe (sales volando)
  JUMP,     // rampa de salto + foso para aterrizar mas abajo
  WHOOPS,   // serie de baches seguidos (te hace rebotar)
  CRATER    // hundimiento (un crater)
};

struct Feature
{
  FeatureKind kind;
  double x;     // donde empieza (pixeles desde el inicio)
  double len;   // cuanto mide de largo
  double h;     // que tan alta/profunda es
};

// TABLA DE OBSTACULOS: {tipo, x, largo, altura}. Ve agregando o cambiando!
const Feature kFeatures[] = {
  {FeatureKind::BUMP,   1100, 380,  60},
  {FeatureKind::RAMP,   1800, 240,  90},
  {FeatureKind::WHOOPS, 2500, 520,  20},
  {FeatureKind::JUMP,   3200, 260, 110},
  {FeatureKind::BUMP,   4100, 400,  80},
  {FeatureKind::CRATER, 4800, 300,  50},
  {FeatureKind::RAMP,   5400, 300, 130},
  {FeatureKind::WHOOPS, 5900, 600,  26},
  {FeatureKind::JUMP,   6700, 300, 120},
  {FeatureKind::BUMP,   7900, 500,  90},
  {FeatureKind::RAMP,   8700, 260, 100},
  {FeatureKind::CRATER, 9200, 360,  60},
  {FeatureKind::WHOOPS, 9800, 600,  28},
  {FeatureKind::JUMP,  10600, 350, 140},
  {FeatureKind::BUMP,  11700, 400, 100},
  {FeatureKind::RAMP,  12400, 300, 120},
  {FeatureKind::WHOOPS, 12900, 600,  30},
  {FeatureKind::JUMP,  13700, 350, 160},
};

// Un punto de masa de la fisica: guarda donde esta AHORA (p) y donde estaba el
// paso ANTERIOR (prev). La velocidad es simplemente la diferencia entre ambos.
// (Esto se llama integracion de Verlet y es muy estable.)
struct Mass
{
  Vec2 p, prev;
  bool contact{false};   // true si esta tocando el suelo
};

enum class DecorKind { TREE, CAR, FIRE };
struct Decor
{
  double x;
  DecorKind kind;
  int seed;
};

class Track
{
public:
  Track()
  {
    const int n = static_cast<int>(TRACK_LEN / STEP_X) + 1;
    pts_.resize(n);
    for (int i = 0; i < n; ++i) {
      const double x = i * STEP_X;
      double y = baseHeight(x);
      for (const Feature & f : kFeatures) {y += featureOffset(f, x);}
      pts_[i] = y;
    }
    buildDecor();
  }

  // Colinas suaves de fondo. El primer tramo (x < 500) es plano para arrancar tranquilo.
  static double baseHeight(double x)
  {
    const double t = std::max(0.0, x - 500.0);
    const double fadeIn = std::min(1.0, t / 1000.0);
    return 430.0 + 0.02 * t +
           fadeIn * (-45.0 * std::sin(t * 0.0035) - 25.0 * std::sin(t * 0.011 + 1.0));
  }

  // Cuanto sube (negativo) o baja (positivo) el terreno por culpa de un obstaculo.
  static double featureOffset(const Feature & f, double x)
  {
    const double u = (x - f.x) / f.len;   // u va de 0 a 1 mientras recorres el obstaculo
    switch (f.kind) {
      case FeatureKind::BUMP: {
          if (u < 0.0 || u > 1.0) {return 0.0;}
          const double s = std::sin(PI * u);
          return -f.h * s * s;
        }
      case FeatureKind::CRATER: {
          if (u < 0.0 || u > 1.0) {return 0.0;}
          const double s = std::sin(PI * u);
          return f.h * s * s;
        }
      case FeatureKind::WHOOPS: {
          if (u < 0.0 || u > 1.0) {return 0.0;}
          const double n = f.len / 90.0;                  // cuantos baches caben
          const double window = std::sin(PI * u);          // suaviza principio y fin
          return -f.h * window * (0.5 - 0.5 * std::cos(2.0 * PI * n * u));
        }
      case FeatureKind::RAMP:
      case FeatureKind::JUMP: {
          if (u < 0.0) {return 0.0;}
          if (u <= 1.0) {return -f.h * std::pow(u, 1.7);}   // la subida (cada vez mas empinada)
          const double d = x - (f.x + f.len);                // pixeles pasada la cima
          if (d <= 30.0) {                                   // caida casi vertical
            double t = d / 30.0;
            t = t * t * (3.0 - 2.0 * t);
            return -f.h * (1.0 - t);
          }
          if (f.kind == FeatureKind::JUMP) {                 // foso para aterrizar
            const double v = (d - 30.0) / 250.0;
            if (v <= 1.0) {
              const double s = std::sin(PI * v);
              return f.h * 0.5 * s * s;
            }
          }
          return 0.0;
        }
    }
    return 0.0;
  }

  // Altura (y) del terreno en cualquier x, uniendo los puntos con rectas.
  double at(double x) const
  {
    const double f = std::clamp(x, 0.0, TRACK_LEN - 0.001) / STEP_X;
    const int i = static_cast<int>(f);
    const double t = f - i;
    return pts_[i] * (1.0 - t) + pts_[i + 1] * t;
  }
  double sample(int i) const {return pts_[std::clamp(i, 0, static_cast<int>(pts_.size()) - 1)];}
  int count() const {return static_cast<int>(pts_.size());}
  const std::vector<Decor> & decor() const {return decor_;}

  // Si la rueda 'w' esta metida dentro del suelo, la saca y le quita la velocidad
  // con la que estaba entrando. Devuelve true si hubo choque.
  bool resolve(Mass & w) const
  {
    const int n = static_cast<int>(pts_.size());
    const int i0 = std::clamp(static_cast<int>(w.p.x / STEP_X) - 4, 0, n - 2);
    const int i1 = std::min(i0 + 8, n - 2);
    bool hit = false;
    for (int i = i0; i <= i1; ++i) {
      const Vec2 a{i * STEP_X, pts_[i]};
      const Vec2 b{(i + 1) * STEP_X, pts_[i + 1]};
      const Vec2 ab = b - a;
      // punto mas cercano del segmento a la rueda
      const double t = std::clamp(dot(w.p - a, ab) / dot(ab, ab), 0.0, 1.0);
      const Vec2 c = a + ab * t;
      const Vec2 d = w.p - c;
      const double dist = len(d);
      if (dist < WHEEL_R) {
        const Vec2 nrm = dist > 1e-6 ? d / dist : Vec2{0.0, -1.0};   // hacia donde empujar
        Vec2 v = w.p - w.prev;
        w.p += nrm * (WHEEL_R - dist);                                 // sacar la rueda
        const double vn = dot(v, nrm);
        if (vn < 0.0) {v -= nrm * vn;}                                 // quitar velocidad hacia el suelo
        w.prev = w.p - v;
        hit = true;
      }
    }
    if (hit) {w.contact = true;}
    return hit;
  }

private:
  // Reparte arboles muertos, autos chocados y barriles con fuego por el camino.
  void buildDecor()
  {
    int k = 0;
    for (double x = 260.0; x < TRACK_LEN - 200.0; x += 140.0 + 260.0 * hash01(k * 3 + 1)) {
      ++k;
      const double slope = (at(x + 20.0) - at(x - 20.0)) / 40.0;
      if (std::fabs(slope) > 0.45) {continue;}       // no poner nada sobre rampas
      const double r = hash01(k * 11 + 5);
      Decor d;
      d.x = x;
      d.seed = k * 97 + 13;
      d.kind = r < 0.50 ? DecorKind::TREE : (r < 0.78 ? DecorKind::CAR : DecorKind::FIRE);
      decor_.push_back(d);
    }
  }

  std::vector<double> pts_;
  std::vector<Decor> decor_;
};

// ---------------------------------------------------------------------------
// 6. Moto: LA FISICA DE LA MOTO XYZ 250
// ---------------------------------------------------------------------------
// La moto es solo DOS PUNTOS (rueda trasera y delantera) unidos por una barra
// rigida. Todo lo demas (cuadro, motor, piloto) se dibuja "pegado" a esos dos
// puntos. Cada paso:
//   1. Aplicamos fuerzas (gravedad, motor, inclinacion).
//   2. Movemos los puntos segun su velocidad.
//   3. Corregimos: mantener la distancia entre ruedas y sacarlas del suelo.

struct Moto
{
  Mass rear, front;                 // rueda trasera y delantera
  double spin_r{0.0}, spin_f{0.0};  // cuanto han girado las ruedas (solo para dibujar)

  // Direccion hacia donde "mira" la moto (de la rueda trasera a la delantera).
  Vec2 axis() const {return normalized(front.p - rear.p);}
  // Direccion "arriba" de la moto (perpendicular al eje).
  Vec2 up() const
  {
    const Vec2 u = axis();
    return {u.y, -u.x};
  }
  Vec2 mid() const {return (rear.p + front.p) * 0.5;}   // punto medio entre ruedas

  // Convierte coordenadas "de la moto" a coordenadas del mundo:
  //   f = cuanto hacia adelante, h = cuanto hacia arriba (desde el centro de la moto).
  // Asi dibujamos todas las piezas sin preocuparnos de que la moto este inclinada.
  Vec2 local(double f, double h) const {return mid() + axis() * f + up() * h;}

  // Velocidad real de la moto en pixeles por segundo.
  Vec2 velocity() const
  {
    const double h = DT / SUBSTEPS;
    return ((rear.p - rear.prev) + (front.p - front.prev)) * (0.5 / h);
  }

  // Le suma un empujon de velocidad (por ejemplo cuando te golpea un monstruo).
  void addVelocity(Vec2 v)
  {
    const double h = DT / SUBSTEPS;
    rear.prev -= v * h;
    front.prev -= v * h;
  }

  // Pone la moto parada sobre el suelo en la posicion x0.
  void reset(const Track & track, double x0)
  {
    rear.p = {x0, track.at(x0) - WHEEL_R - 1.0};
    front.p = {x0 + WHEEL_BASE, track.at(x0 + WHEEL_BASE) - WHEEL_R - 1.0};
    rear.prev = rear.p;
    front.prev = front.p;
    rear.contact = front.contact = false;
    spin_r = spin_f = 0.0;
  }

  // Mueve un punto: nueva_pos = pos + velocidad + aceleracion * dt^2
  static void integrate(Mass & q, Vec2 a, double h)
  {
    const Vec2 v = (q.p - q.prev) * DAMPING;
    q.prev = q.p;
    q.p = q.p + v + a * (h * h);
  }

  // Obliga a que las ruedas siempre estén a WHEEL_BASE de distancia.
  void constrain()
  {
    const Vec2 d = front.p - rear.p;
    const double dist = std::max(len(d), 1e-6);
    const Vec2 corr = d * ((dist - WHEEL_BASE) / dist * 0.5);
    rear.p += corr;
    front.p -= corr;
  }

  // Un paso completo de fisica.
  //   throttle: 0..1 acelerar    lean: -1 atras, +1 adelante    brake: 0..1 freno
  void step(const Track & track, double dt, double throttle, double lean, double brake)
  {
    const double h = dt / SUBSTEPS;
    for (int s = 0; s < SUBSTEPS; ++s) {
      const Vec2 u = axis();
      const Vec2 w = up();
      Vec2 ar{0.0, GRAVITY};    // aceleracion rueda trasera (la gravedad tira hacia abajo)
      Vec2 af{0.0, GRAVITY};    // aceleracion rueda delantera

      // El motor empuja SOLO si la rueda trasera toca el suelo (tiene traccion).
      if (rear.contact) {
        const double speed = dot(rear.p - rear.prev, u) / h;
        if (throttle < 0.0 || speed < MAX_DRIVE_SPEED) {
          ar += u * (DRIVE * throttle);
        }
      }
      // Inclinar = empujar una rueda hacia arriba y la otra hacia abajo (hace girar la moto).
      ar += w * (LEAN * lean);
      af -= w * (LEAN * lean);

      // Freno: reduce la velocidad de ambas ruedas si tocan el suelo.
      if (brake > 0.0 && (rear.contact || front.contact)) {
        // brake=1 quita 4% de velocidad por paso (freno normal); valores menores frenan menos.
        rear.prev = rear.p - (rear.p - rear.prev) * (1.0 - 0.04 * brake);
        front.prev = front.p - (front.p - front.prev) * (1.0 - 0.04 * brake);
      }

      integrate(rear, ar, h);
      integrate(front, af, h);

      rear.contact = front.contact = false;
      for (int i = 0; i < ITERATIONS; ++i) {
        constrain();
        track.resolve(rear);
        track.resolve(front);
      }

      // Giro visual de las ruedas (para animar los rayos).
      const Vec2 u2 = axis();
      spin_r += dot(rear.p - rear.prev, u2) / WHEEL_R;
      spin_f += dot(front.p - front.prev, u2) / WHEEL_R;
    }
  }
};

// ---------------------------------------------------------------------------
// 7. PILOTO, MONSTRUOS, BALAS, BALONES, PICKUPS Y PARTICULAS
// ---------------------------------------------------------------------------

// Posicion de cada parte del cuerpo del piloto (se recalcula en cada cuadro).
struct Pose
{
  Vec2 hip, knee, foot, shoulder, head;
  Vec2 elbowBar, handBar;   // brazo que sujeta el manubrio
  Vec2 elbowGun, handGun;   // brazo que sostiene el arma
  Vec2 gunTip;              // punta del cañon (de aqui salen las balas)
};

// Calcula la pose del piloto segun como esta la moto y hacia donde apunta.
Pose makePose(const Moto & m, Vec2 aim)
{
  Pose p;
  p.hip = m.local(-20, 38);        // cadera sentada en el asiento
  p.shoulder = m.local(-9, 64);    // hombro
  p.head = m.local(-3, 77);        // centro del casco
  p.foot = m.local(-4, 8);         // pie sobre el estribo
  p.knee = m.local(14, 30);        // rodilla (hacia adelante)
  p.handBar = m.local(34, 54);     // mano izquierda en el manubrio
  p.elbowBar = (p.shoulder + p.handBar) * 0.5 + m.up() * -7.0;

  // Brazo del arma: sale del hombro y se estira hacia donde apuntas.
  const double side = aim.x >= 0.0 ? 1.0 : -1.0;
  const Vec2 down{-aim.y * side, aim.x * side};   // "abajo" respecto al arma
  p.handGun = p.shoulder + aim * 20.0 + down * 2.0;
  p.elbowGun = p.shoulder + aim * 11.0 + down * 5.0;
  p.gunTip = p.handGun + aim * 38.0;
  return p;
}

// ZOMBI es el tipo "personaje" (se mantiene el nombre del enum para no tener
// que cambiar el resto del codigo). Cual personaje es lo decide el campo
// 'variant' de Monster.
// BRUTO es el monstruo grande: ahora es la "ESTRELLA POP" (cantante ficticio).
enum class MonsterType { ZOMBI, MURCIELAGO, BRUTO };

struct Spawn            // "aqui va a aparecer un monstruo"
{
  double x;
  MonsterType type;
};

struct Monster
{
  MonsterType type{MonsterType::ZOMBI};
  int variant{0};         // solo para ZOMBI: 0=Escobar 1=Gengis Kan 2=Hitler 3=Messi
  Vec2 p;                 // posicion (personaje/bruto: pies; murcielago: centro)
  double hp{1.0}, maxHp{1.0};
  double speed{60.0};
  double anim{0.0};       // reloj para animar piernas/alas
  double hurt{0.0};       // tiempo restante del parpadeo blanco al recibir un balazo
  double facing{-1.0};    // -1 mira a la izquierda, +1 a la derecha
  bool alive{true};
  bool active{false};     // false = dormido (todavia no te ve)
  int spawnId{0};
  double kickT{0.0};      // (Messi) tiempo restante de la animacion de patada
  double kickCd{1.0};     // (Messi) cuenta regresiva para la proxima patada
};

struct Bullet
{
  Vec2 p, v;
  double life{1.0};       // segundos que le quedan de vida
};

// Balon de futbol que patea Messi. Vuela en arco (le afecta la gravedad) y rebota.
struct Ball
{
  Vec2 p, v;
  double life{6.0};       // segundos que le quedan de vida
  double spin{0.0};       // cuanto ha girado (solo para dibujar)
};

enum class PickupKind { AMMO, MEDKIT };
struct Pickup
{
  Vec2 p;
  PickupKind kind;
  bool taken{false};
};

// Particula visual: chispas, polvo, humo, sangre... No afectan la jugabilidad.
struct Fx
{
  Vec2 p, v;
  double life{1.0}, maxLife{1.0};
  double size{3.0};
  double gravity{0.0};
  double drag{0.0};
  bool grow{false};       // true = se hace mas grande al morir (humo)
  Color color{255, 255, 255, 255};
};

// ----- Piloto caido (ragdoll con fisica Verlet) -----
// Al morir, el piloto deja de ser un dibujo pegado a la moto y se convierte en 9
// PUNTOS (cadera, hombro, cabeza, rodilla, pie, codos y manos) unidos por BARRAS
// de largo fijo (huesos). Cada punto cae, rebota y se arrastra por el suelo por su
// cuenta, y las barras los mantienen unidos: asi se ve un cuerpo flojo que da
// vueltas de verdad. Es la misma idea que usamos en la moto (struct Moto).
constexpr int RAG_N = 9;      // cuantos puntos tiene el cuerpo
constexpr int RAG_SUB = 2;    // cada cuadro se divide en 2 pasos de fisica (mas precision)
enum RagPart {R_HIP, R_SHOULDER, R_HEAD, R_KNEE, R_FOOT, R_ELBOW_BAR, R_HAND_BAR, R_ELBOW_GUN, R_HAND_GUN};
struct RagStick       // un "hueso": une los puntos a y b y mantiene su distancia
{
  int a, b;
  double len;
};

// Datos de cada tipo de monstruo.
double monsterScale(MonsterType t) {return t == MonsterType::BRUTO ? 1.8 : 1.0;}
double monsterRadius(MonsterType t)
{
  switch (t) {
    case MonsterType::ZOMBI: return 20.0;
    case MonsterType::MURCIELAGO: return 15.0;
    case MonsterType::BRUTO: return 36.0;
  }
  return 20.0;
}
Vec2 monsterCenter(const Monster & m)    // centro del cuerpo (donde pegan las balas)
{
  if (m.type == MonsterType::MURCIELAGO) {return m.p;}
  return {m.p.x, m.p.y - 26.0 * monsterScale(m.type)};
}
// true si el monstruo es Messi (el unico que patea balones).
inline bool isMessi(const Monster & m) {return m.type == MonsterType::ZOMBI && m.variant == 3;}

// ---------------------------------------------------------------------------
// 8. Game: LOGICA DEL JUEGO Y DIBUJADO
// ---------------------------------------------------------------------------
// Esta clase NO sabe nada de ROS: solo usa SDL. Asi funciona igual con o sin ROS.

class Game
{
public:
  Game()
  {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
      throw std::runtime_error(std::string("SDL_Init fallo: ") + SDL_GetError());
    }
    win_ = SDL_CreateWindow(
      "MOTO XYZ 250 - APOCALIPSIS", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, 0);
    if (!win_) {
      throw std::runtime_error(std::string("SDL_CreateWindow fallo: ") + SDL_GetError());
    }
    // Intenta usar la tarjeta grafica; si no se puede, dibuja por software.
    ren_ = SDL_CreateRenderer(win_, -1, SDL_RENDERER_ACCELERATED);
    if (!ren_) {ren_ = SDL_CreateRenderer(win_, -1, SDL_RENDERER_SOFTWARE);}
    if (!ren_) {
      throw std::runtime_error(std::string("SDL_CreateRenderer fallo: ") + SDL_GetError());
    }
    SDL_SetRenderDrawBlendMode(ren_, SDL_BLENDMODE_BLEND);   // activa transparencias
    SDL_ShowCursor(SDL_DISABLE);                              // usamos nuestra propia mira
    openFirstPad();                                           // si hay un control conectado, lo abre

    gfx_ = Gfx(ren_);
    buildWorld();
    restartAll();
  }

  ~Game()
  {
    if (pad_) {SDL_GameControllerClose(pad_);}
    if (ren_) {SDL_DestroyRenderer(ren_);}
    if (win_) {SDL_DestroyWindow(win_);}
    SDL_Quit();
  }

  // Un cuadro del juego. Devuelve false cuando hay que cerrar el programa.
  bool tick()
  {
    if (!handleEvents()) {return false;}
    readInput();
    update();
    render();
    return true;
  }

  // ----- Funciones para controlar el juego desde ROS -----
  void setRemoteControl(double throttle, double lean)
  {
    remote_throttle_ = std::clamp(throttle, -1.0, 1.0);
    remote_lean_ = std::clamp(lean, -1.0, 1.0);
    last_cmd_ = std::chrono::steady_clock::now();
  }
  void setRemoteFire(bool fire)
  {
    remote_fire_ = fire;
    last_fire_cmd_ = std::chrono::steady_clock::now();
  }

  // ----- Datos para publicar en ROS -----
  int score() const {return score_;}
  int health() const {return static_cast<int>(std::max(0.0, health_));}
  Vec2 motoPos() const {return moto_.mid();}
  double motoYaw() const
  {
    const Vec2 d = moto_.front.p - moto_.rear.p;
    return std::atan2(-d.y, d.x);
  }

  // Guarda lo que se ve en pantalla como imagen BMP.
  bool saveScreenshot(const char * path)
  {
    SDL_Surface * s = SDL_CreateRGBSurfaceWithFormat(0, WIN_W, WIN_H, 32, SDL_PIXELFORMAT_ARGB8888);
    if (!s) {return false;}
    SDL_RenderReadPixels(ren_, nullptr, SDL_PIXELFORMAT_ARGB8888, s->pixels, s->pitch);
    const bool ok = SDL_SaveBMP(s, path) == 0;
    SDL_FreeSurface(s);
    return ok;
  }

private:
  enum class State { RUNNING, CRASHED, FINISHED };

  // Lo que el jugador esta pidiendo en este momento.
  struct Input
  {
    double throttle{0.0}, lean{0.0}, brake{0.0};
    bool fire{false};
  };

  // ======================= CREAR EL MUNDO ==============================

  // Se llama UNA vez al iniciar: decide donde van los monstruos y los pickups.
  void buildWorld()
  {
    // Monstruos: cada ~300-560 px aparece uno. Mas adelante hay mas variedad.
    int k = 0;
    for (double x = 1000.0; x < FINISH_X - 300.0; x += 300.0 + 260.0 * hash01(k * 7 + 2)) {
      ++k;
      const double progress = x / FINISH_X;
      const double r = hash01(k * 13 + 9);
      Spawn s;
      s.x = x;
      if (progress < 0.15) {
        s.type = MonsterType::ZOMBI;                     // al inicio solo personajes (facil)
      } else if (r < 0.45) {
        s.type = MonsterType::ZOMBI;
      } else if (r < 0.85) {
        s.type = MonsterType::MURCIELAGO;
      } else {
        s.type = progress > 0.35 ? MonsterType::BRUTO : MonsterType::ZOMBI;
      }
      spawns_.push_back(s);
    }
    spawn_dead_.assign(spawns_.size(), 0);

    // Cajas de balas cada 950 px y botiquines cada 1900 px.
    for (double x = 800.0; x < FINISH_X - 200.0; x += 950.0) {
      pickups_.push_back({{x, track_.at(x) - 48.0}, PickupKind::AMMO, false});
    }
    for (double x = 1700.0; x < FINISH_X - 200.0; x += 1900.0) {
      pickups_.push_back({{x, track_.at(x) - 48.0}, PickupKind::MEDKIT, false});
    }
  }

  // Reinicia TODO (nueva partida desde cero).
  void restartAll()
  {
    score_ = 0;
    kills_ = 0;
    health_ = MAX_HEALTH;
    ammo_ = START_AMMO;
    checkpoint_x_ = 0.0;
    std::fill(spawn_dead_.begin(), spawn_dead_.end(), 0);
    for (Pickup & p : pickups_) {p.taken = false;}
    hint_time_ = 12.0;                 // mostrar ayuda de controles 12 segundos
    startRun(120.0);
  }

  // Coloca la moto en x0 y vuelve a crear los monstruos que quedan por delante.
  void startRun(double x0)
  {
    moto_.reset(track_, x0);
    bullets_.clear();
    balls_.clear();                    // quitar los balones que estaban volando
    fx_.clear();
    monsters_.clear();
    for (size_t i = 0; i < spawns_.size(); ++i) {
      if (spawn_dead_[i] || spawns_[i].x < x0 + 500.0) {continue;}
      monsters_.push_back(makeMonster(static_cast<int>(i)));
    }
    state_ = State::RUNNING;
    state_timer_ = 0.0;
    invuln_ = 1.5;
    hurt_flash_ = 0.0;
    muzzle_t_ = 0.0;
    fire_cd_ = 0.0;
    banner_.clear();
    banner_t_ = 0.0;
    // Camara: saltar directo a la moto (sin deslizarse).
    cam_ = {moto_.mid().x - WIN_W * 0.33, moto_.mid().y - WIN_H * 0.58};
    updateAimAndPose();
  }

  Monster makeMonster(int spawnId) const
  {
    const Spawn & s = spawns_[spawnId];
    Monster m;
    m.type = s.type;
    m.spawnId = spawnId;
    const double r = hash01(spawnId * 29 + 3);
    switch (s.type) {
      case MonsterType::ZOMBI:
        m.hp = m.maxHp = 3.0;
        m.speed = 60.0 + 40.0 * r;
        m.p = {s.x, track_.at(s.x)};
        // Elige el personaje segun el spawn (siempre el mismo en cada partida).
        m.variant = std::min(3, static_cast<int>(hash01(spawnId * 53 + 7) * 4.0));
        m.kickCd = 0.8 + 0.8 * r;      // Messi: primera patada poco despues de despertar
        break;
      case MonsterType::MURCIELAGO:
        m.hp = m.maxHp = 2.0;
        m.speed = 200.0 + 60.0 * r;
        m.p = {s.x, track_.at(s.x) - 170.0};
        break;
      case MonsterType::BRUTO:
        m.hp = m.maxHp = 10.0;
        m.speed = 48.0 + 12.0 * r;
        m.p = {s.x, track_.at(s.x)};
        break;
    }
    m.anim = r * 10.0;
    return m;
  }

  // ======================= ENTRADA (TECLADO / MOUSE) ===================

  // ======================= CHOQUE REALISTA (PILOTO + MOTO) ==============
  // QUE PASA EN UN CHOQUE REAL A VELOCIDAD?
  //   1. INERCIA: el piloto lleva la misma velocidad que la moto. Si la moto se
  //      frena o cae, el cuerpo NO se frena: sale disparado hacia adelante.
  //   2. Vuela en arco (gravedad) y da vueltas, porque lo que lo empuja no pasa
  //      por su centro.
  //   3. Al tocar el suelo rebota poco y se ARRASTRA: el roce lo va frenando
  //      poco a poco, levantando polvo.
  //   4. La moto, sin piloto, tambien se vuelca, rueda y se arrastra sacando
  //      CHISPAS del metal contra el suelo, hasta detenerse.
  // Todo eso esta simulado en las funciones de este bloque.

  // Se llama UNA vez, justo en el momento de morir.
  void startRagdoll()
  {
    const Pose & p = pose_;
    const Vec2 pts[RAG_N] = {p.hip, p.shoulder, p.head, p.knee, p.foot, p.elbowBar, p.handBar, p.elbowGun, p.handGun};

    // INERCIA: cada punto arranca con la velocidad de la moto...
    const Vec2 v0 = moto_.velocity();
    const double speed = len(v0);
    // ...mas un giro: mientras mas rapido ibas, mas vueltas da (omega en radianes/s).
    const double omega = std::clamp(speed / 120.0, 1.5, 7.0);
    const double h = DT / RAG_SUB;
    for (int i = 0; i < RAG_N; ++i) {
      const Vec2 r = pts[i] - p.hip;                             // posicion respecto a la cadera
      const Vec2 vi = v0 * 0.92 + Vec2{-r.y, r.x} * omega + Vec2{0.0, -120.0};   // velocidad + giro + pequeno salto
      rag_[i].p = pts[i];
      rag_[i].prev = pts[i] - vi * h;                            // Verlet: velocidad = p - prev
      rag_[i].contact = false;
    }

    // Los "huesos": largo inicial = distancia que tenian en la pose sentada.
    ragSticks_.clear();
    const int links[][2] = {
      {R_HIP, R_SHOULDER}, {R_SHOULDER, R_HEAD}, {R_HIP, R_HEAD},           // torso y cuello (el triangulo lo mantiene rigido)
      {R_HIP, R_KNEE}, {R_KNEE, R_FOOT},                                     // pierna
      {R_SHOULDER, R_ELBOW_BAR}, {R_ELBOW_BAR, R_HAND_BAR},                  // brazo del manubrio
      {R_SHOULDER, R_ELBOW_GUN}, {R_ELBOW_GUN, R_HAND_GUN}};                 // brazo del arma
    for (const auto & l : links) {
      ragSticks_.push_back({l[0], l[1], len(pts[l[0]] - pts[l[1]])});
    }

    // Para dibujar el casco y la bota bien girados guardamos con que angulo quedo
    // el torso respecto a la moto (ver riderAxis()).
    const Vec2 w0 = normalized(p.shoulder - p.hip);
    const Vec2 u0{-w0.y, w0.x};
    ragBase_ = std::atan2(moto_.axis().y, moto_.axis().x) - std::atan2(u0.y, u0.x);

    // La MOTO tambien se descontrola: un empujon hacia arriba en la rueda delantera
    // la hace dar una voltereta (mas fuerte mientras mas rapido ibas).
    moto_.front.prev.y += std::clamp(speed * 0.0015, 0.4, 1.8);

    crash_frames_ = 0;
    ragHitCd_ = 0.0;
    buildRagPose();
  }

  // Copia la posicion de los 9 puntos a 'pose_' (lo que usa el dibujo del piloto).
  void buildRagPose()
  {
    pose_.hip = rag_[R_HIP].p;
    pose_.shoulder = rag_[R_SHOULDER].p;
    pose_.head = rag_[R_HEAD].p;
    pose_.knee = rag_[R_KNEE].p;
    pose_.foot = rag_[R_FOOT].p;
    pose_.elbowBar = rag_[R_ELBOW_BAR].p;
    pose_.handBar = rag_[R_HAND_BAR].p;
    pose_.elbowGun = rag_[R_ELBOW_GUN].p;
    pose_.handGun = rag_[R_HAND_GUN].p;
    // El rifle sigue la direccion del antebrazo (del codo a la mano).
    aim_ = normalized(pose_.handGun - pose_.elbowGun);
    pose_.gunTip = pose_.handGun + aim_ * 38.0;
  }

  // Velocidad del cuerpo en px/s (la sacamos de la cadera).
  Vec2 ragVelocity() const
  {
    return (rag_[R_HIP].p - rag_[R_HIP].prev) * (RAG_SUB / DT);
  }

  // Un cuadro de fisica del piloto caido.
  void updateRagdoll()
  {
    // Radio de cada punto (cuanto "ocupa" contra el suelo): la cabeza es la mas grande.
    static const double kRadius[RAG_N] = {9.0, 8.0, 10.0, 6.0, 5.0, 4.0, 4.0, 4.0, 5.0};
    const double h = DT / RAG_SUB;
    ragHitCd_ = std::max(0.0, ragHitCd_ - DT);
    double impact = 0.0;          // el golpe mas fuerte contra el suelo en este cuadro (px/s)
    Vec2 impactAt{0.0, 0.0};

    for (int sub = 0; sub < RAG_SUB; ++sub) {
      // 1) Mover cada punto con su velocidad y la gravedad.
      for (Mass & q : rag_) {
        const Vec2 v = (q.p - q.prev) * 0.9995;
        q.prev = q.p;
        q.p = q.p + v + Vec2{0.0, GRAVITY} * (h * h);
      }
      // 2) Mantener los huesos a su largo y sacar los puntos del suelo (varias pasadas).
      for (int it = 0; it < 6; ++it) {
        for (const RagStick & st : ragSticks_) {
          const Vec2 d = rag_[st.b].p - rag_[st.a].p;
          const double dist = std::max(len(d), 1e-6);
          const Vec2 corr = d * ((dist - st.len) / dist * 0.5);
          rag_[st.a].p += corr;
          rag_[st.b].p -= corr;
        }
        for (int i = 0; i < RAG_N; ++i) {
          const double gy = track_.at(rag_[i].p.x);
          if (rag_[i].p.y + kRadius[i] > gy) {rag_[i].p.y = gy - kRadius[i];}
        }
      }
      // 3) Respuesta al contacto: rebote suave y ROCE (lo que lo frena al arrastrarse).
      for (int i = 0; i < RAG_N; ++i) {
        Mass & q = rag_[i];
        const double gy = track_.at(q.p.x);
        q.contact = q.p.y + kRadius[i] >= gy - 0.5;
        if (!q.contact) {continue;}
        const Vec2 v = q.p - q.prev;
        if (v.y > 0.3) {                                  // llego con fuerza: rebota un 20%
          if (v.y / h > impact) {impact = v.y / h; impactAt = {q.p.x, gy};}
          q.prev.y = q.p.y + v.y * 0.2;
        } else {
          q.prev.y = q.p.y;                               // apoyado: sin temblor
        }
        q.prev.x = q.p.x - v.x * 0.99;                   // roce: pierde 1% de velocidad por paso
      }
    }

    // Golpe fuerte contra el suelo: nube de polvo y temblor de camara.
    if (impact > 300.0 && ragHitCd_ <= 0.0) {
      emit(impactAt, 10, -PI / 2.0, 1.2, 180.0, rgb(150, 115, 80, 190), 0.7, 7.0, 70.0, 1.5, true);
      shake_ = std::max(shake_, std::min(10.0, impact * 0.012));
      ragHitCd_ = 0.25;
    }
    // Arrastrandose: polvo continuo mientras se mueve rapido.
    const double speed = len(ragVelocity());
    if (speed > 150.0) {
      for (int i : {R_HIP, R_SHOULDER, R_HEAD, R_FOOT}) {
        if (rag_[i].contact && rnd(0.0, 1.0) < 0.35) {
          emit({rag_[i].p.x, track_.at(rag_[i].p.x)}, 1, -PI * 0.75, 0.6, 80.0 + speed * 0.1, rgb(140, 105, 75, 170), 0.6, 6.0, 40.0, 1.5, true);
        }
      }
    }
    buildRagPose();
  }

  // Chispas del metal de la moto contra el suelo mientras se arrastra.
  void crashSparks()
  {
    const Vec2 vel = moto_.velocity();
    const double speed = len(vel);
    if (speed < 100.0) {return;}
    const Vec2 back = normalized(vel * -1.0);                          // las chispas salen hacia atras...
    const double ang = std::atan2(back.y, back.x) + (back.x < 0.0 ? 0.6 : -0.6);   // ...y un poco hacia arriba
    const Mass * wheels[2] = {&moto_.rear, &moto_.front};
    for (const Mass * w : wheels) {
      if (w->contact && rnd(0.0, 1.0) < 0.7) {
        emit(w->p + Vec2{0.0, WHEEL_R}, 2, ang, 0.5, 200.0 + speed * 0.3, rgb(255, 210, 100), 0.35, 2.0, 600.0);
      }
    }
  }

  // Direccion "adelante" y "arriba" del piloto: sigue a la moto, o al cuerpo si se cayo.
  Vec2 riderAxis() const
  {
    if (state_ != State::CRASHED) {return moto_.axis();}
    const Vec2 w = normalized(pose_.shoulder - pose_.hip);             // "arriba" del torso
    return rotated(Vec2{-w.y, w.x}, ragBase_);
  }
  Vec2 riderUp() const
  {
    const Vec2 u = riderAxis();
    return {u.y, -u.x};
  }

  // Busca entre los dispositivos conectados el primero que sea un "game controller"
  // (un control con botones estandar, como el de Xbox 360) y lo abre.
  void openFirstPad()
  {
    if (pad_) {return;}
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
      if (SDL_IsGameController(i)) {
        pad_ = SDL_GameControllerOpen(i);
        if (pad_) {return;}
      }
    }
  }

  // Un stick/gatillo da un numero entre -32768 y 32767. Lo convertimos a -1..1.
  double padRaw(SDL_GameControllerAxis a) const
  {
    return std::clamp(SDL_GameControllerGetAxis(pad_, a) / 32767.0, -1.0, 1.0);
  }
  // Igual, pero con "zona muerta": si el stick esta casi quieto devuelve 0.
  double padAxis(SDL_GameControllerAxis a) const
  {
    const double v = padRaw(a);
    return std::fabs(v) < PAD_DEADZONE ? 0.0 : v;
  }

  bool handleEvents()
  {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
        return false;
      }
      if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r) {restartAll();}
      if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F12) {saveScreenshot("captura.bmp");}
      if (e.type == SDL_MOUSEMOTION) {mouse_seen_ = true; aimFromPad_ = false;}   // mover el mouse devuelve la punteria al mouse
      // Control conectado o desconectado mientras el juego esta abierto.
      if (e.type == SDL_CONTROLLERDEVICEADDED) {openFirstPad();}
      if (e.type == SDL_CONTROLLERDEVICEREMOVED && pad_ &&
        SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(pad_)) == e.cdevice.which)
      {
        SDL_GameControllerClose(pad_);
        pad_ = nullptr;
        openFirstPad();                      // por si hay otro conectado
      }
      if (e.type == SDL_CONTROLLERBUTTONDOWN && e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
        restartAll();
      }
    }
    return true;
  }

  void readInput()
  {
    const Uint8 * k = SDL_GetKeyboardState(nullptr);   // estado actual de todas las teclas
    in_ = Input{};
    if (k[SDL_SCANCODE_UP] || k[SDL_SCANCODE_W]) {in_.throttle += 1.0;}
    if (k[SDL_SCANCODE_RIGHT] || k[SDL_SCANCODE_D]) {in_.lean += 1.0;}
    if (k[SDL_SCANCODE_LEFT] || k[SDL_SCANCODE_A]) {in_.lean -= 1.0;}
    if (k[SDL_SCANCODE_DOWN] || k[SDL_SCANCODE_S]) {in_.brake = 1.0;}

    int mx = 0, my = 0;
    const Uint32 buttons = SDL_GetMouseState(&mx, &my);
    mouse_ = {static_cast<double>(mx), static_cast<double>(my)};
    if ((buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) || k[SDL_SCANCODE_SPACE]) {in_.fire = true;}

    // ----- Control de Xbox 360 -----
    if (pad_) {
      if (SDL_GameControllerGetButton(pad_, SDL_CONTROLLER_BUTTON_A)) {in_.fire = true;}       // A = disparar
      if (SDL_GameControllerGetButton(pad_, SDL_CONTROLLER_BUTTON_B)) {in_.brake = 1.0;}       // B = frenar
      // RT = acelerar. El gatillo es un eje de 0 a 32767: lo usamos como acelerador
      // "analogico" (apretado a la mitad = acelera la mitad).
      if (SDL_GameControllerGetAxis(pad_, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > PAD_TRIGGER_ON) {
        in_.throttle += SDL_GameControllerGetAxis(pad_, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0;
      }
      // Stick izquierdo, eje X = inclinar la moto (derecha = adelante, izquierda = atras).
      in_.lean += padAxis(SDL_CONTROLLER_AXIS_LEFTX);
      // Stick derecho = direccion del arma. Usamos la distancia al centro (radial)
      // para la zona muerta. Si lo sueltas, el arma se queda apuntando donde estaba.
      const Vec2 st{padRaw(SDL_CONTROLLER_AXIS_RIGHTX), padRaw(SDL_CONTROLLER_AXIS_RIGHTY)};
      if (len(st) > 0.35) {
        padAim_ = normalized(st);      // en SDL "abajo" es +y, igual que en nuestro juego
        aimFromPad_ = true;
      }
    }

    // Comandos de ROS: solo valen si llegaron hace menos de medio segundo.
    const auto now = std::chrono::steady_clock::now();
    if (now - last_cmd_ < std::chrono::milliseconds(500)) {
      in_.throttle += remote_throttle_;
      in_.lean += remote_lean_;
    }
    if (remote_fire_ && now - last_fire_cmd_ < std::chrono::milliseconds(500)) {in_.fire = true;}
    in_.throttle = std::clamp(in_.throttle, -1.0, 1.0);
    in_.lean = std::clamp(in_.lean, -1.0, 1.0);
  }

  // ======================= ACTUALIZAR EL MUNDO ==========================

  double rnd(double a, double b) {return std::uniform_real_distribution<double>(a, b)(rng_);}

  // Crea 'count' particulas en 'pos'. 'angle' = direccion (en radianes, -PI/2 = arriba),
  // 'spread' = que tan abierto es el abanico, 'speed' = velocidad, 'life' = duracion (s).
  void emit(
    Vec2 pos, int count, double angle, double spread, double speed, Color col, double life,
    double size, double gravity, double drag = 0.0, bool grow = false)
  {
    for (int i = 0; i < count && fx_.size() < MAX_FX; ++i) {
      Fx f;
      const double a = angle + rnd(-spread, spread);
      const double s = speed * rnd(0.3, 1.0);
      f.p = pos;
      f.v = {std::cos(a) * s, std::sin(a) * s};
      f.life = f.maxLife = life * rnd(0.6, 1.0);
      f.size = size * rnd(0.6, 1.2);
      f.gravity = gravity;
      f.drag = drag;
      f.grow = grow;
      f.color = col;
      fx_.push_back(f);
    }
  }

  void update()
  {
    // CAMARA LENTA en el momento del choque: durante los primeros 45 cuadros
    // (~0.75 s) solo avanzamos la fisica un cuadro de cada dos.
    if (state_ == State::CRASHED && crash_frames_ < 45) {
      ++crash_frames_;
      if (crash_frames_ % 2 == 1) {return;}
    }
    time_ += DT;
    invuln_ = std::max(0.0, invuln_ - DT);
    hurt_flash_ = std::max(0.0, hurt_flash_ - DT);
    muzzle_t_ = std::max(0.0, muzzle_t_ - DT);
    fire_cd_ = std::max(0.0, fire_cd_ - DT);
    banner_t_ = std::max(0.0, banner_t_ - DT);
    no_ammo_t_ = std::max(0.0, no_ammo_t_ - DT);
    hint_time_ = std::max(0.0, hint_time_ - DT);

    if (state_ == State::RUNNING) {
      stepMoto();
    } else if (state_ == State::CRASHED) {
      moto_.step(track_, DT, 0.0, 0.0, 0.15);     // la moto sigue sola, sin motor y con las ruedas rozando
      crashSparks();
    }
    if (state_ == State::CRASHED) {updateRagdoll();} else {updateAimAndPose();}

    if (state_ == State::RUNNING) {
      handleShooting();
      updateBullets();
      updateMonsters();
      updateBalls();
      collectPickups();
      updateCheckpoints();
      checkEnd();
    } else {
      // Muerto o ganaste: esperamos un poco y luego reaparecemos.
      state_timer_ -= DT;
      if (state_timer_ <= 0.0) {
        if (state_ == State::FINISHED) {
          restartAll();
        } else {
          respawn();
        }
      }
    }
    updateFx();
    updateCamera();
  }

  // Despues de morir: volver al ultimo checkpoint con vida llena.
  void respawn()
  {
    health_ = MAX_HEALTH;
    ammo_ = std::max(ammo_, 30);
    startRun(checkpoint_x_ > 0.0 ? checkpoint_x_ : 120.0);
  }

  void stepMoto()
  {
    const Vec2 vBefore = moto_.velocity();
    const bool wasAir = !(moto_.rear.contact || moto_.front.contact);

    moto_.step(track_, DT, in_.throttle, in_.lean, in_.brake);

    const bool onGround = moto_.rear.contact || moto_.front.contact;
    // Aterrizaje fuerte: nube de polvo y temblor de camara.
    if (wasAir && onGround && vBefore.y > 450.0) {
      const Vec2 where = moto_.mid() + Vec2{0.0, WHEEL_R};
      emit(where, 18, -PI / 2.0, 1.2, 260.0, rgb(150, 115, 80, 200), 0.8, 8.0, 80.0, 2.0, true);
      shake_ = std::max(shake_, std::min(12.0, vBefore.y * 0.012));
    }
    const double speed = len(moto_.velocity());
    // Polvo que levanta la rueda trasera.
    if (moto_.rear.contact && speed > 120.0 && rnd(0.0, 1.0) < 0.6) {
      const Vec2 where = moto_.rear.p + Vec2{0.0, WHEEL_R};
      emit(where, 1, -PI * 0.75, 0.5, 120.0 + speed * 0.1, rgb(140, 105, 75, 170), 0.6, 6.0, 40.0, 1.5, true);
    }
    // Humo del escape cuando aceleras.
    if (in_.throttle > 0.0 && rnd(0.0, 1.0) < 0.5) {
      const Vec2 tip = moto_.local(-62, 14);
      emit(tip, 1, std::atan2(-moto_.axis().y, -moto_.axis().x), 0.3, 90.0, rgb(110, 110, 115, 150), 0.7, 4.0, -30.0, 1.0, true);
    }
  }

  // Calcula hacia donde apunta el arma y la pose completa del piloto.
  void updateAimAndPose()
  {
    const Vec2 shoulder = moto_.local(-9, 64);
    Vec2 aim = moto_.axis();      // por defecto, al frente
    if (aimFromPad_) {
      aim = padAim_;              // el stick izquierdo manda
    } else if (mouse_seen_) {
      // La mira esta en pantalla; sumamos la camara para saber donde cae en el mundo.
      const Vec2 target{mouse_.x + cam_.x, mouse_.y + cam_.y};
      const Vec2 d = target - shoulder;
      if (len(d) > 8.0) {aim = d / len(d);}
    }
    aim_ = aim;
    pose_ = makePose(moto_, aim_);
  }

  void handleShooting()
  {
    if (!in_.fire || fire_cd_ > 0.0) {return;}
    if (ammo_ <= 0) {
      fire_cd_ = 0.25;
      no_ammo_t_ = 0.6;           // muestra "SIN BALAS!"
      return;
    }
    fire_cd_ = FIRE_DELAY;
    --ammo_;
    const Vec2 dir = rotated(aim_, rnd(-0.035, 0.035));   // pequeña dispersion
    Bullet b;
    b.p = pose_.gunTip;
    b.v = dir * BULLET_SPEED + moto_.velocity();          // la bala hereda la velocidad de la moto
    b.life = 1.2;
    bullets_.push_back(b);
    muzzle_t_ = 0.06;
    shake_ = std::max(shake_, 2.5);
    emit(pose_.gunTip, 3, std::atan2(dir.y, dir.x), 0.4, 300.0, rgb(255, 210, 100), 0.2, 2.0, 0.0);
  }

  void updateBullets()
  {
    for (Bullet & b : bullets_) {
      const Vec2 prev = b.p;
      b.p += b.v * DT;
      b.life -= DT;
      // Choca con el suelo?
      if (b.p.y > track_.at(b.p.x)) {
        emit(b.p, 5, -PI / 2.0, 1.4, 160.0, rgb(255, 200, 90), 0.3, 2.0, 400.0);
        emit(b.p, 3, -PI / 2.0, 1.0, 80.0, rgb(130, 100, 75, 180), 0.5, 5.0, 0.0, 1.0, true);
        b.life = 0.0;
        continue;
      }
      // Choca con un BALON de Messi? Lo destruimos (+5 puntos).
      bool hitBall = false;
      for (Ball & ba : balls_) {
        if (ba.life <= 0.0) {continue;}
        if (distPointSegment(ba.p, prev, b.p) < BALL_R + 4.0) {
          ba.life = 0.0;
          score_ += 5;
          emit(ba.p, 10, 0.0, PI, 240.0, rgb(255, 255, 255), 0.5, 3.0, 400.0);
          emit(ba.p, 6, 0.0, PI, 200.0, rgb(30, 30, 34), 0.5, 3.0, 400.0);
          b.life = 0.0;
          hitBall = true;
          break;
        }
      }
      if (hitBall) {continue;}
      // Choca con un monstruo? (miramos TODO el trayecto de la bala este cuadro)
      for (Monster & m : monsters_) {
        if (!m.alive || !m.active) {continue;}
        if (distPointSegment(monsterCenter(m), prev, b.p) < monsterRadius(m.type)) {
          damageMonster(m, 1.0);
          m.p.x += (b.v.x > 0 ? 1.0 : -1.0) * (m.type == MonsterType::MURCIELAGO ? 8.0 : 3.0);
          b.life = 0.0;
          break;
        }
      }
    }
    bullets_.erase(
      std::remove_if(bullets_.begin(), bullets_.end(), [](const Bullet & b) {return b.life <= 0.0;}),
      bullets_.end());
  }

  // Color de la "sangre" de cada monstruo (en la Estrella Pop sale confeti rosa).
  Color slimeColor(MonsterType t) const
  {
    if (t == MonsterType::MURCIELAGO) {return rgb(150, 40, 170);}
    if (t == MonsterType::BRUTO) {return rgb(235, 90, 200);}
    return rgb(110, 200, 50);
  }

  // Le quita vida a un monstruo; si llega a 0 muere y sumas puntos.
  void damageMonster(Monster & m, double dmg)
  {
    m.hp -= dmg;
    m.hurt = 0.12;
    const Vec2 c = monsterCenter(m);
    emit(c, 6, 0.0, PI, 220.0, slimeColor(m.type), 0.6, 3.5, 500.0);
    if (m.hp <= 0.0) {killMonster(m);}
  }

  void killMonster(Monster & m)
  {
    m.alive = false;
    spawn_dead_[m.spawnId] = 1;
    ++kills_;
    switch (m.type) {
      case MonsterType::ZOMBI: score_ += 10; break;
      case MonsterType::MURCIELAGO: score_ += 15; break;
      case MonsterType::BRUTO: score_ += 50; break;
    }
    const Vec2 c = monsterCenter(m);
    const int n = m.type == MonsterType::BRUTO ? 40 : 22;
    emit(c, n, 0.0, PI, 320.0, slimeColor(m.type), 0.9, 5.0, 600.0);
    emit(c, n / 2, 0.0, PI, 200.0, rgb(70, 20, 20), 0.9, 4.0, 600.0);
    shake_ = std::max(shake_, m.type == MonsterType::BRUTO ? 10.0 : 4.0);
  }

  // true si el monstruo (circulo en 'center') toca alguna parte de la moto o del piloto.
  bool touchesPlayer(Vec2 center, double radius) const
  {
    const Vec2 pts[] = {moto_.mid(), moto_.front.p, moto_.rear.p, pose_.head, pose_.hip};
    for (const Vec2 & p : pts) {
      if (len(p - center) < radius + 14.0) {return true;}
    }
    return false;
  }

  // El jugador recibe dano de un monstruo en la posicion 'from'.
  void hurtPlayer(double dmg, Vec2 from)
  {
    if (invuln_ > 0.0) {return;}                // tras un golpe eres inmune un momento
    health_ -= dmg;
    invuln_ = 1.1;
    hurt_flash_ = 0.35;
    shake_ = std::max(shake_, 9.0);
    const Vec2 away = normalized(moto_.mid() - from);
    moto_.addVelocity({away.x * 170.0, -170.0});    // empujon suave hacia atras y arriba
    emit(moto_.mid(), 14, 0.0, PI, 260.0, rgb(190, 25, 25), 0.7, 4.0, 600.0);
  }

  // ----- Messi patea un balon hacia el jugador -----
  // Es un "tiro parabolico": elegimos un tiempo de vuelo T y calculamos la
  // velocidad inicial (vx, vy) para que el balon llegue a donde ESTARA la moto
  // dentro de T segundos (le adelantamos un poco la posicion, como un buen pase).
  //   x(T) = x0 + vx*T                 ->  vx = (xf - x0) / T
  //   y(T) = y0 + vy*T + g*T^2 / 2     ->  vy = (yf - y0 - g*T^2/2) / T
  void messiKick(Monster & m)
  {
    m.kickT = 0.35;                                       // arranca la animacion de patada
    const Vec2 start{m.p.x + m.facing * 24.0, m.p.y - 12.0};   // a la altura del pie
    const double T = std::clamp(std::fabs(moto_.mid().x - start.x) / 500.0, 0.55, 1.4);
    const Vec2 target = moto_.mid() + moto_.velocity() * (T * 0.8);   // donde estara la moto
    Ball b;
    b.p = start;
    b.v = {(target.x - start.x) / T, (target.y - start.y - 0.5 * BALL_GRAVITY * T * T) / T};
    b.life = 6.0;
    balls_.push_back(b);
    // Polvito de la patada.
    emit(start, 6, -PI / 2.0, 1.0, 120.0, rgb(150, 115, 80, 190), 0.5, 4.0, 60.0, 1.5, true);
  }

  // Mueve los balones: gravedad, rebote en el suelo y choque con el jugador.
  void updateBalls()
  {
    for (Ball & b : balls_) {
      b.v.y += BALL_GRAVITY * DT;
      b.p += b.v * DT;
      b.spin += b.v.x * DT * 0.05;
      b.life -= DT;

      // Rebote en el suelo: invierte la velocidad vertical y pierde algo de energia.
      const double gy = track_.at(b.p.x);
      if (b.p.y + BALL_R > gy) {
        b.p.y = gy - BALL_R;
        if (b.v.y > 0.0) {b.v.y = -b.v.y * 0.55;}
        b.v.x *= 0.92;
        emit({b.p.x, gy}, 2, -PI / 2.0, 0.8, 70.0, rgb(150, 115, 80, 170), 0.4, 3.0, 40.0, 1.5, true);
      }

      // Si ya quedo muy atras de la moto, lo borramos.
      if (b.p.x < moto_.mid().x - 1500.0) {b.life = 0.0;}

      // Golpe al jugador (si estas en periodo de inmunidad el balon te atraviesa).
      if (b.life > 0.0 && invuln_ <= 0.0 && touchesPlayer(b.p, BALL_R)) {
        hurtPlayer(DAMAGE_BALL, b.p);
        emit(b.p, 8, 0.0, PI, 220.0, rgb(255, 255, 255), 0.5, 3.0, 400.0);
        b.life = 0.0;
      }
    }
    balls_.erase(
      std::remove_if(balls_.begin(), balls_.end(), [](const Ball & b) {return b.life <= 0.0;}),
      balls_.end());
  }

  void updateMonsters()
  {
    const Vec2 target = moto_.mid();
    const double motoSpeed = len(moto_.velocity());
    for (Monster & m : monsters_) {
      if (!m.alive) {continue;}
      m.hurt = std::max(0.0, m.hurt - DT);
      m.kickT = std::max(0.0, m.kickT - DT);

      // Dormido: se despierta cuando la moto se acerca.
      if (!m.active) {
        if (std::fabs(m.p.x - target.x) < ACTIVATE_DIST) {m.active = true;} else {continue;}
      }
      // Quedo muy atras: lo borramos para no gastar recursos.
      if (m.p.x < target.x - 1600.0) {m.alive = false; continue;}

      const double dir = target.x > m.p.x ? 1.0 : -1.0;    // hacia donde esta el jugador
      m.facing = dir;

      if (m.type == MonsterType::MURCIELAGO) {
        // Vuela hacia ti subiendo y bajando como una ola.
        m.anim += DT * 14.0;
        m.p.x += dir * m.speed * DT;
        const double wantedY = target.y - 70.0 + 55.0 * std::sin(time_ * 2.2 + m.spawnId);
        m.p.y += (wantedY - m.p.y) * std::min(1.0, 3.0 * DT);
        m.p.y = std::min(m.p.y, track_.at(m.p.x) - 20.0);   // no atravesar el suelo
      } else if (isMessi(m)) {
        // MESSI: se acerca hasta MESSI_STAND_DIST, se queda parado y te patea balones.
        const double dist = std::fabs(target.x - m.p.x);
        if (dist > MESSI_STAND_DIST) {
          m.anim += DT * m.speed * 0.12;                    // camina
          m.p.x += dir * m.speed * DT;
        }
        m.p.y = track_.at(m.p.x);
        m.kickCd -= DT;
        if (m.kickCd <= 0.0 && dist < MESSI_KICK_RANGE) {
          messiKick(m);
          m.kickCd = MESSI_KICK_DELAY + rnd(0.0, 0.9);      // espera antes de la siguiente
        }
      } else {
        // Camina por el suelo hacia ti, pegado al terreno.
        m.anim += DT * m.speed * 0.12;
        m.p.x += dir * m.speed * DT;
        m.p.y = track_.at(m.p.x);
      }

      // Contacto con el jugador.
      if (touchesPlayer(monsterCenter(m), monsterRadius(m.type))) {
        if (m.type == MonsterType::ZOMBI && motoSpeed > RUN_OVER_SPEED) {
          killMonster(m);                         // atropellado! (tambien Messi)
        } else if (isMessi(m)) {
          // Messi no hace dano al tocarlo: su ataque son los balones.
        } else if (m.type == MonsterType::MURCIELAGO) {
          hurtPlayer(DAMAGE_BAT, m.p);
          emit(m.p, 10, 0.0, PI, 220.0, slimeColor(m.type), 0.6, 3.5, 500.0);
          m.alive = false;                         // el murcielago se estrella y muere
          spawn_dead_[m.spawnId] = 1;
        } else {
          hurtPlayer(m.type == MonsterType::BRUTO ? DAMAGE_BRUTO : DAMAGE_ZOMBI, m.p);
          m.p.x -= dir * 50.0;                     // el monstruo retrocede tras golpear
        }
      }
    }
    monsters_.erase(
      std::remove_if(monsters_.begin(), monsters_.end(), [](const Monster & m) {return !m.alive;}),
      monsters_.end());
  }

  void collectPickups()
  {
    const Vec2 m = moto_.mid();
    for (Pickup & p : pickups_) {
      if (p.taken || len(p.p - m) > 55.0) {continue;}
      p.taken = true;
      if (p.kind == PickupKind::AMMO) {
        ammo_ = std::min(MAX_AMMO, ammo_ + 15);
        banner_ = "+15 BALAS";
      } else {
        health_ = std::min(MAX_HEALTH, health_ + 35.0);
        banner_ = "+35 VIDA";
      }
      banner_t_ = 1.0;
      emit(p.p, 14, -PI / 2.0, PI, 200.0, p.kind == PickupKind::AMMO ? rgb(255, 220, 80) : rgb(90, 255, 120), 0.7, 3.0, 100.0);
    }
  }

  // Posicion de cada checkpoint: 3800, 7600, 11400...
  static double checkpointX(int i) {return (i + 1) * CHECKPOINT_GAP;}

  void updateCheckpoints()
  {
    for (int i = 0; checkpointX(i) < FINISH_X; ++i) {
      const double cx = checkpointX(i);
      if (moto_.mid().x > cx && cx > checkpoint_x_) {
        checkpoint_x_ = cx;
        banner_ = "CHECKPOINT!";
        banner_t_ = 2.0;
      }
    }
  }

  void checkEnd()
  {
    const bool headHit = pose_.head.y + 9.0 > track_.at(pose_.head.x);
    const bool hipHit = pose_.hip.y + 6.0 > track_.at(pose_.hip.x);
    if (health_ <= 0.0) {
      state_ = State::CRASHED;
      state_timer_ = 3.0;
      death_msg_ = "TE MATARON!";
      startRagdoll();
      emit(pose_.hip, 40, 0.0, PI, 380.0, rgb(190, 25, 25), 1.0, 5.0, 700.0);
      shake_ = 16.0;
    } else if (headHit || hipHit || moto_.rear.p.x < 0.0) {
      state_ = State::CRASHED;
      state_timer_ = 3.0;
      death_msg_ = "TE CAISTE!";
      startRagdoll();
      health_ = 0.0;
      emit(pose_.head, 30, -PI / 2.0, 1.4, 350.0, rgb(255, 200, 90), 0.8, 2.5, 700.0);
      emit(pose_.head, 20, -PI / 2.0, 1.4, 250.0, rgb(150, 115, 80, 200), 1.0, 9.0, 80.0, 1.5, true);
      shake_ = 16.0;
    } else if (moto_.mid().x > FINISH_X) {
      state_ = State::FINISHED;
      state_timer_ = 6.0;
      score_ += static_cast<int>(health_);              // bono por la vida que te queda
    }
  }

  void updateFx()
  {
    for (Fx & f : fx_) {
      f.v.y += f.gravity * DT;
      f.v = f.v * std::max(0.0, 1.0 - f.drag * DT);
      f.p += f.v * DT;
      f.life -= DT;
    }
    fx_.erase(std::remove_if(fx_.begin(), fx_.end(), [](const Fx & f) {return f.life <= 0.0;}), fx_.end());

    // Chispas que suben de los barriles con fuego que estan cerca.
    for (const Decor & d : track_.decor()) {
      if (d.kind != DecorKind::FIRE) {continue;}
      if (std::fabs(d.x - moto_.mid().x) > 900.0) {continue;}
      if (rnd(0.0, 1.0) < 0.25) {
        emit({d.x + rnd(-6, 6), track_.at(d.x) - 38.0}, 1, -PI / 2.0, 0.4, 70.0, rgb(255, 160, 50), 1.2, 2.0, -20.0);
      }
    }
  }

  void updateCamera()
  {
    // Al chocar, la camara sigue al piloto (que es lo que sale volando).
    const bool crashed = state_ == State::CRASHED;
    const Vec2 m = crashed ? rag_[R_HIP].p : moto_.mid();
    const Vec2 v = crashed ? ragVelocity() : moto_.velocity();
    const double lookAhead = std::clamp(v.x * 0.12, -60.0, 120.0);   // mira un poco adelante
    const double tx = m.x - WIN_W * 0.33 + lookAhead;
    const double ty = m.y - WIN_H * 0.58;
    cam_.x += (tx - cam_.x) * 0.12;      // la camara "persigue" suavemente a la moto
    cam_.y += (ty - cam_.y) * 0.08;
    shake_ *= 0.88;                      // el temblor se va apagando
    if (shake_ < 0.1) {shake_ = 0.0;}
    shakeOff_ = {rnd(-1.0, 1.0) * shake_, rnd(-1.0, 1.0) * shake_};
  }

  // ======================= DIBUJAR ====================================
  // Orden de dibujado (lo que se dibuja primero queda ATRAS):
  //   cielo -> sol -> nubes -> edificios lejanos -> terreno -> decoracion ->
  //   pickups -> monstruos -> balones -> moto y piloto -> balas -> particulas ->
  //   cenizas -> vineta -> HUD (vida, balas, puntos...)

  void render()
  {
    const Vec2 cam = cam_ + shakeOff_;     // camara con temblor
    drawSky();
    drawBackground(cam);

    gfx_.setView(cam);
    drawGround();
    drawDecor();
    drawCheckpointsAndFinish();
    drawPickups();
    drawMonsters();
    drawBalls();
    drawHeadlight();
    drawMoto();
    drawBulletsAndFx();
    drawDeathSpeech();

    gfx_.setView({0.0, 0.0});
    drawEmbers(cam);
    drawVignette();
    drawHud();
    SDL_RenderPresent(ren_);
  }

  // ---- Cielo, sol, nubes y edificios del fondo (efecto "parallax") ----
  // Parallax: las cosas lejanas se mueven MAS LENTO que la camara, y asi
  // parece que hay profundidad.

  void drawSky()
  {
    gfx_.setView({0.0, 0.0});
    // Dos degradados uno encima del otro dan un cielo de atardecer rojo.
    gfx_.vGradient(0, 0, WIN_W, WIN_H * 0.5, rgb(16, 8, 30), rgb(105, 32, 55));
    gfx_.vGradient(0, WIN_H * 0.5, WIN_W, WIN_H * 0.5 + 1, rgb(105, 32, 55), rgb(240, 130, 55));

    // Sol rojo gigante con resplandor.
    const Vec2 sun{WIN_W * 0.68 - cam_.x * 0.02, 290.0 - (cam_.y - 70.0) * 0.05};
    gfx_.glow(sun, 300.0, rgb(255, 110, 40, 110), rgb(255, 110, 40, 0));
    gfx_.circle(sun, 76.0, rgb(255, 120, 50), 40);
    gfx_.circle(sun, 56.0, rgb(255, 190, 100), 40);

    // Nubes de humo que se arrastran lentamente.
    for (int i = 0; i < 7; ++i) {
      const double x = wrapTo(i * 330.0 + time_ * 7.0 - cam_.x * 0.05, WIN_W + 700.0) - 350.0;
      const double y = 70.0 + 190.0 * hash01(i * 5 + 1) - (cam_.y - 70.0) * 0.05;
      const Color c = rgb(55, 20, 34, 140);
      gfx_.ellipse({x, y}, 130.0, 24.0, c);
      gfx_.ellipse({x + 70.0, y - 14.0}, 95.0, 22.0, c);
      gfx_.ellipse({x - 80.0, y - 8.0}, 85.0, 20.0, c);
    }
  }

  // Dibuja una fila de edificios en ruinas. 'f' = factor de parallax (0..1).
  void drawSkyline(
    Vec2 cam, double f, double base, double cell, double hmin, double hmax, Color col, int seed,
    bool windows)
  {
    const double offx = cam.x * f;
    const double offy = (cam.y - 70.0) * f;
    const int i0 = static_cast<int>(std::floor(offx / cell)) - 1;
    const int i1 = static_cast<int>(std::floor((offx + WIN_W) / cell)) + 2;
    for (int i = i0; i <= i1; ++i) {
      const double h = hmin + hash01(i * 7 + seed) * (hmax - hmin);
      const double w = cell * (0.6 + 0.35 * hash01(i * 13 + seed));
      const double x = i * cell - offx + (cell - w) * 0.5;
      const double top = base - offy - h;
      const double slant = (hash01(i * 17 + seed) - 0.5) * h * 0.25;   // techo roto e inclinado
      gfx_.polygon({{x, WIN_H + 200.0}, {x, top + slant}, {x + w, top - slant}, {x + w, WIN_H + 200.0}}, col);
      if (hash01(i * 23 + seed) > 0.6) {                               // antena
        gfx_.line({x + w * 0.5, top}, {x + w * 0.5, top - 25.0}, 2.0, col);
      }
      if (windows) {                                                    // ventanas encendidas
        for (int r = 0; r < 6; ++r) {
          for (int c = 0; c < 3; ++c) {
            if (hash01(i * 101 + r * 7 + c * 3 + seed) > 0.88) {
              const double wx = x + 6.0 + c * (w - 14.0) / 3.0;
              const double wy = top + 14.0 + r * 18.0;
              if (wy < base - offy - 6.0) {gfx_.rect(wx, wy, 5.0, 7.0, rgb(255, 170, 60, 170));}
            }
          }
        }
      }
    }
  }

  void drawBackground(Vec2 cam)
  {
    gfx_.setView({0.0, 0.0});
    drawSkyline(cam, 0.08, 370.0, 90.0, 110.0, 250.0, rgb(98, 40, 60), 11, false);   // mas lejos
    drawSkyline(cam, 0.20, 395.0, 76.0, 90.0, 210.0, rgb(64, 28, 46), 37, false);
    drawSkyline(cam, 0.40, 420.0, 64.0, 60.0, 160.0, rgb(36, 18, 30), 71, true);     // mas cerca
  }

  // ---- Terreno ----
  void drawGround()
  {
    const Vec2 v = gfx_.view();
    // Dibujamos un poco mas alla de los bordes de la pista (sample() repite la
    // altura del primer/ultimo punto), asi el suelo nunca se corta a la vista.
    const int i0 = static_cast<int>(std::floor(v.x / STEP_X)) - 1;
    const int i1 = static_cast<int>((v.x + WIN_W) / STEP_X) + 2;
    const double bottom = v.y + WIN_H + 40.0;

    std::vector<Vec2> top, bot, topLow, botLow;
    for (int i = i0; i <= i1; ++i) {
      const double x = i * STEP_X;
      const double y = track_.sample(i);
      top.push_back({x, y});
      bot.push_back({x, bottom});
      topLow.push_back({x, y + 34.0});
      botLow.push_back({x, y + 40.0});
    }
    // Tierra seca con degradado (clara arriba, oscura abajo).
    gfx_.strip(top, bot, rgb(104, 80, 62), rgb(30, 22, 24));
    // Franja de roca mas oscura.
    gfx_.strip(topLow, botLow, rgb(20, 14, 16, 90), rgb(20, 14, 16, 90));
    // Borde superior: costra de ceniza oscura y luego un filo de polvo claro.
    std::vector<Vec2> crustTop, crustBot, dustTop, dustBot;
    for (int i = i0; i <= i1; ++i) {
      const double x = i * STEP_X;
      const double y = track_.sample(i);
      crustTop.push_back({x, y - 1.0});
      crustBot.push_back({x, y + 9.0});
      dustTop.push_back({x, y - 3.0});
      dustBot.push_back({x, y});
    }
    gfx_.strip(crustTop, crustBot, rgb(52, 38, 34), rgb(52, 38, 34, 0));
    gfx_.strip(dustTop, dustBot, rgb(190, 150, 105), rgb(190, 150, 105));

    // Piedritas sueltas sobre el suelo.
    for (int i = i0; i <= i1; i += 5) {
      if (hash01(i * 5 + 3) < 0.45) {
        const double x = i * STEP_X + 3.0;
        const double s = 2.5 + 5.0 * hash01(i * 9 + 1);
        gfx_.polygon({{x - s, track_.at(x) + 2.0}, {x - s * 0.5, track_.at(x) - s}, {x + s * 0.6, track_.at(x) - s * 0.7}, {x + s, track_.at(x) + 2.0}}, rgb(74, 60, 56));
      }
    }
  }

  // ---- Decoracion: arboles muertos, autos oxidados, barriles con fuego ----
  void drawBranch(Vec2 base, double angle, double length, double thick, int depth, int seed)
  {
    const Vec2 tip = base + Vec2{std::cos(angle), std::sin(angle)} * length;
    gfx_.line(base, tip, thick, rgb(26, 18, 20));
    if (depth <= 0) {return;}
    for (int k = 0; k < 2; ++k) {                     // cada rama se divide en 2
      const double a = angle + (k == 0 ? -1.0 : 1.0) * (0.35 + 0.5 * hash01(seed + k * 31 + depth * 7));
      const double l = length * (0.62 + 0.22 * hash01(seed * 3 + k + depth));
      drawBranch(tip, a, l, thick * 0.66, depth - 1, seed * 5 + k + 1);
    }
  }

  void drawDecor()
  {
    const Vec2 v = gfx_.view();
    for (const Decor & d : track_.decor()) {
      if (d.x < v.x - 150.0 || d.x > v.x + WIN_W + 150.0) {continue;}   // fuera de pantalla
      const double gy = track_.at(d.x);
      switch (d.kind) {
        case DecorKind::TREE:
          drawBranch({d.x, gy + 3.0}, -PI / 2.0 + (hash01(d.seed) - 0.5) * 0.3,
            48.0 + 22.0 * hash01(d.seed + 1), 9.0, 4, d.seed);
          break;
        case DecorKind::CAR: drawWreck(d); break;
        case DecorKind::FIRE: drawFireBarrel(d); break;
      }
    }
  }

  void drawWreck(const Decor & d)
  {
    // El auto se inclina siguiendo la pendiente del suelo.
    const double gy = track_.at(d.x);
    const Vec2 u = normalized({40.0, track_.at(d.x + 20.0) - track_.at(d.x - 20.0)});
    const Vec2 w{u.y, -u.x};
    const Vec2 base{d.x, gy};
    auto L = [&](double f, double h) {return base + u * f + w * h;};
    const bool rusty = hash01(d.seed + 5) > 0.5;
    const Color body = rusty ? rgb(70, 40, 34) : rgb(42, 38, 46);   // oscuros para no confundirse con la moto
    const Color dark = rgb(22, 18, 22);

    gfx_.polygon({L(-58, 10), L(58, 10), L(60, 32), L(42, 38), L(-58, 38)}, body);          // carroceria
    gfx_.polygon({L(-30, 38), L(-18, 62), L(22, 62), L(38, 38)}, mix(body, dark, 0.2));      // cabina
    gfx_.polygon({L(-24, 40), L(-16, 58), L(-2, 58), L(-2, 40)}, dark);                      // ventana rota
    gfx_.polygon({L(2, 40), L(2, 58), L(18, 58), L(30, 40)}, dark);
    gfx_.line(L(-40, 22), L(-10, 20), 3.0, mix(body, rgb(200, 90, 40), 0.35));               // oxido
    gfx_.circle(L(-34, 13), 13.0, rgb(15, 15, 17), 14);                                       // ruedas
    gfx_.circle(L(-34, 13), 6.0, rgb(80, 80, 85), 10);
    if (hash01(d.seed + 9) > 0.4) {                                                           // la otra rueda falta a veces
      gfx_.circle(L(36, 13), 13.0, rgb(15, 15, 17), 14);
      gfx_.circle(L(36, 13), 6.0, rgb(80, 80, 85), 10);
    }
  }

  void drawFireBarrel(const Decor & d)
  {
    const double gy = track_.at(d.x);
    const double t = time_ * 13.0 + d.seed;
    gfx_.glow({d.x, gy - 50.0}, 110.0, rgb(255, 140, 40, 90), rgb(255, 140, 40, 0));        // luz del fuego
    gfx_.polygon({{d.x - 12, gy}, {d.x + 12, gy}, {d.x + 12, gy - 34}, {d.x - 12, gy - 34}}, rgb(70, 66, 72));
    gfx_.rect(d.x - 12, gy - 22, 24, 3, rgb(40, 38, 44));
    gfx_.rect(d.x - 12, gy - 10, 24, 3, rgb(40, 38, 44));
    // Tres capas de llama que parpadean.
    const double h1 = 34.0 + 10.0 * std::sin(t);
    const double h2 = 24.0 + 8.0 * std::sin(t * 1.3 + 1.0);
    const double h3 = 14.0 + 5.0 * std::sin(t * 1.7 + 2.0);
    gfx_.polygon({{d.x - 11, gy - 34}, {d.x + 11, gy - 34}, {d.x + 3.0 * std::sin(t * 0.7), gy - 34 - h1}}, rgb(240, 90, 20, 230));
    gfx_.polygon({{d.x - 8, gy - 34}, {d.x + 8, gy - 34}, {d.x + 2.0 * std::sin(t * 0.9), gy - 34 - h2}}, rgb(255, 170, 40, 240));
    gfx_.polygon({{d.x - 5, gy - 34}, {d.x + 5, gy - 34}, {d.x, gy - 34 - h3}}, rgb(255, 235, 140));
  }

  // ---- Checkpoints y meta ----
  void drawCheckpointsAndFinish()
  {
    const Vec2 v = gfx_.view();
    for (int i = 0; checkpointX(i) < FINISH_X; ++i) {
      const double x = checkpointX(i);
      if (x < v.x - 100.0 || x > v.x + WIN_W + 100.0) {continue;}
      const double gy = track_.at(x);
      const bool passed = checkpoint_x_ >= x;
      const Color flag = passed ? rgb(70, 220, 100) : rgb(230, 80, 50);
      gfx_.glow({x + 25, gy - 100}, 70.0, fade(flag, 0.45), fade(flag, 0.0));
      gfx_.line({x, gy}, {x, gy - 115}, 5.0, rgb(150, 150, 158));
      const double wave = std::sin(time_ * 4.0) * 4.0;
      gfx_.polygon({{x, gy - 115}, {x + 56, gy - 100 + wave}, {x, gy - 82}}, flag);
    }
    // Arco de meta con cuadros blancos y negros.
    const double gy = track_.at(FINISH_X);
    gfx_.line({FINISH_X - 90, gy}, {FINISH_X - 90, gy - 170}, 8.0, rgb(160, 160, 168));
    gfx_.line({FINISH_X + 90, gy}, {FINISH_X + 90, gy - 170}, 8.0, rgb(160, 160, 168));
    for (int r = 0; r < 3; ++r) {
      for (int c = 0; c < 15; ++c) {
        gfx_.rect(FINISH_X - 90 + c * 12.0, gy - 170 + r * 12.0, 12.0, 12.0, ((r + c) % 2) ? rgb(250, 250, 250) : rgb(20, 20, 20));
      }
    }
    gfx_.text("META", FINISH_X - 60, gy - 135, 5, rgb(255, 255, 255));
  }

  // ---- Pickups (balas y botiquines) ----
  void drawPickups()
  {
    const Vec2 v = gfx_.view();
    for (const Pickup & p : pickups_) {
      if (p.taken || p.p.x < v.x - 60.0 || p.p.x > v.x + WIN_W + 60.0) {continue;}
      const Vec2 c = p.p + Vec2{0.0, std::sin(time_ * 3.0 + p.p.x * 0.01) * 5.0};   // flota
      if (p.kind == PickupKind::AMMO) {
        gfx_.glow(c, 44.0, rgb(255, 210, 60, 110), rgb(255, 210, 60, 0));
        gfx_.polygon({{c.x - 14, c.y - 10}, {c.x + 14, c.y - 10}, {c.x + 14, c.y + 10}, {c.x - 14, c.y + 10}}, rgb(60, 70, 40));
        gfx_.rect(c.x - 12, c.y - 8, 24, 16, rgb(190, 160, 50));
        for (int k = 0; k < 3; ++k) {                                               // 3 balas dibujadas
          gfx_.rect(c.x - 9 + k * 8, c.y - 5, 5, 9, rgb(230, 190, 90));
          gfx_.polygon({{c.x - 9 + k * 8, c.y - 5}, {c.x - 4 + k * 8, c.y - 5}, {c.x - 6.5 + k * 8, c.y - 10}}, rgb(160, 90, 40));
        }
      } else {
        gfx_.glow(c, 44.0, rgb(90, 255, 120, 110), rgb(90, 255, 120, 0));
        gfx_.rect(c.x - 14, c.y - 11, 28, 22, rgb(235, 235, 235));
        gfx_.rect(c.x - 3, c.y - 8, 6, 16, rgb(210, 30, 30));                       // cruz roja
        gfx_.rect(c.x - 8, c.y - 3, 16, 6, rgb(210, 30, 30));
      }
    }
  }

  // ---- Monstruos ----
  void drawMonsters()
  {
    const Vec2 v = gfx_.view();
    for (const Monster & m : monsters_) {
      if (!m.alive || m.p.x < v.x - 150.0 || m.p.x > v.x + WIN_W + 150.0) {continue;}
      if (m.type == MonsterType::MURCIELAGO) {drawBat(m);} else {drawZombie(m);}
      // Nombre del personaje sobre la cabeza.
      if (m.type == MonsterType::ZOMBI) {
        static const char * kNames[] = {"PABLO ESCOBAR", "GENGIS KAN", "HITLER", "MESSI"};
        gfx_.textCentered(kNames[m.variant], m.p.x, m.p.y - 105.0, 2, rgb(255, 255, 255));
      } else if (m.type == MonsterType::BRUTO) {
        gfx_.textCentered("ESTRELLA POP", m.p.x, m.p.y - 150.0, 2, rgb(255, 150, 230));
      }
      // Barra de vida (solo si ya recibio dano).
      if (m.hp < m.maxHp) {
        const Vec2 c = monsterCenter(m);
        const double top = c.y - monsterRadius(m.type) - 18.0 * monsterScale(m.type);
        const double w = 34.0 * monsterScale(m.type);
        gfx_.rect(c.x - w / 2 - 1, top - 1, w + 2, 6, rgb(0, 0, 0, 180));
        gfx_.rect(c.x - w / 2, top, w * (m.hp / m.maxHp), 4, rgb(220, 50, 40));
      }
    }
  }

  // Los 4 personajes humanos y la Estrella Pop (monstruo grande) usan este dibujo.
  void drawZombie(const Monster & m)
  {
    const bool brute = m.type == MonsterType::BRUTO;   // 'brute' = la Estrella Pop
    const int vr = brute ? -1 : m.variant;   // 0 Escobar, 1 Gengis Kan, 2 Hitler, 3 Messi
    const double s = monsterScale(m.type);
    const double fd = m.facing;
    // L(f,h): convierte "f hacia adelante, h hacia arriba" del monstruo a coordenadas del mundo.
    auto L = [&](double f, double h) {return Vec2{m.p.x + f * fd * s, m.p.y - h * s};};

    // Colores segun el personaje.
    Color skin, cloth, pants;
    switch (vr) {
      case 0: skin = rgb(205, 160, 125); cloth = rgb(230, 230, 235); pants = rgb(40, 40, 50); break;   // Escobar: camisa blanca
      case 1: skin = rgb(200, 150, 110); cloth = rgb(130, 50, 40);   pants = rgb(70, 50, 36); break;   // Gengis Kan: tunica roja
      case 2: skin = rgb(225, 185, 160); cloth = rgb(95, 95, 80);    pants = rgb(35, 35, 40); break;   // Hitler: uniforme gris
      case 3: skin = rgb(215, 170, 135); cloth = rgb(245, 245, 250); pants = rgb(25, 25, 30); break;   // Messi: camiseta albiceleste
      default: skin = rgb(235, 200, 175); cloth = rgb(150, 60, 205); pants = rgb(24, 22, 30); break;   // Estrella Pop: chamarra morada brillante
    }
    if (m.hurt > 0.0) {            // parpadeo blanco al recibir un balazo
      skin = mix(skin, rgb(255, 255, 255), 0.7);
      cloth = mix(cloth, rgb(255, 255, 255), 0.7);
      pants = mix(pants, rgb(255, 255, 255), 0.7);
    }
    const double ph = m.anim;
    const double swing = std::sin(ph) * 10.0;                    // vaiven de piernas
    const double bob = std::fabs(std::sin(ph)) * 2.0;            // sube y baja al caminar
    const double th = brute ? 1.5 : 1.0;                          // grosor

    // Luz de escenario rosa detras de la Estrella Pop.
    if (brute) {
      gfx_.glow(L(4, 40), 70.0 * s, rgb(255, 90, 210, 90), rgb(255, 90, 210, 0), 20);
    }

    // piernas (la pierna delantera de Messi se estira al patear el balon)
    Vec2 footFront = L(swing, 1);
    if (vr == 3 && m.kickT > 0.0) {
      const double k = std::sin(std::clamp(m.kickT / 0.35, 0.0, 1.0) * PI);   // 0 -> 1 -> 0
      footFront = L(6.0 + 24.0 * k, 1.0 + 12.0 * k);
    }
    gfx_.line(L(0, 22), L(-swing, 1), 7.0 * s * th, mix(pants, rgb(0, 0, 0), 0.3), true);
    gfx_.line(L(0, 22), footFront, 7.0 * s * th, pants, true);
    gfx_.line(footFront, footFront + Vec2{7.0 * fd * s, 0.0}, 5.0 * s * th, rgb(30, 26, 30), true);

    // torso
    gfx_.line(L(0, 22), L(3, 44 + bob), 15.0 * s * th, cloth, true);
    if (brute) {
      // Lentejuelas brillantes en la chamarra.
      for (int k = 0; k < 5; ++k) {
        const double tw = 0.5 + 0.5 * std::sin(time_ * 6.0 + k * 1.7);
        gfx_.circle(L(0.5 + (k % 2) * 3.0, 26 + k * 4.0 + bob), 1.6 * s, rgb(255, 235, 140, static_cast<int>(120 + 135 * tw)), 6);
      }
      gfx_.line(L(-4, 30 + bob), L(8, 32 + bob), 2.5 * s, rgb(255, 120, 220));   // franja rosa
    }
    if (vr == 3) {                                    // rayas celestes de la camiseta de Messi
      for (int k = -1; k <= 1; ++k) {
        gfx_.line(L(0.5 + k * 4.5, 24), L(3.5 + k * 4.5, 43 + bob), 3.0 * s, rgb(120, 195, 240));
      }
    }
    if (vr == 1) {                                    // cinturon de Gengis Kan
      gfx_.line(L(-6, 31 + bob), L(8, 33 + bob), 3.0 * s, rgb(230, 190, 60));
    }
    // brazos
    const double armWave = std::sin(ph * 0.5) * 3.0;
    if (brute) {
      // Un brazo sostiene el microfono frente a la boca; el otro saluda hacia arriba.
      gfx_.line(L(4, 42 + bob), L(16, 52 + bob), 5.5 * s * th, skin, true);
      gfx_.line(L(4, 40 + bob), L(22, 52 + armWave * 3.0 + 6.0), 5.5 * s * th, mix(skin, rgb(0, 0, 0), 0.25), true);
      gfx_.line(L(16, 52 + bob), L(20, 55 + bob), 2.0 * s, rgb(70, 70, 76), true);   // mango del microfono
      gfx_.circle(L(21, 57 + bob), 3.2 * s, rgb(190, 190, 200), 10);                 // cabeza del microfono
    } else {
      // brazos estirados hacia adelante
      gfx_.line(L(4, 42 + bob), L(26, 41 + armWave), 5.5 * s * th, skin, true);
      gfx_.line(L(4, 40 + bob), L(23, 33 - armWave), 5.5 * s * th, mix(skin, rgb(0, 0, 0), 0.25), true);
    }
    // cabeza
    const Vec2 head = L(8, 55 + bob);
    gfx_.circle(head, 9.5 * s, skin, 16);

    if (brute) {
      // Estrella Pop: pelo oscuro con flequillo, ojo normal y boca abierta cantando.
      gfx_.circle(L(6, 62 + bob), 8.0 * s, rgb(28, 22, 30), 12);
      gfx_.polygon({L(9, 63 + bob), L(18, 59 + bob), L(17, 56 + bob), L(8, 58 + bob)}, rgb(28, 22, 30));
      gfx_.circle(L(13, 57 + bob), 1.8 * s, rgb(15, 15, 18), 8);                  // ojo
      gfx_.circle(L(14, 50 + bob), 2.4 * s, rgb(90, 20, 40), 8);                  // boca abierta
      gfx_.circle(L(3, 53 + bob), 1.6 * s, rgb(255, 235, 140), 6);                // arete brillante
      return;
    }

    // ojo normal para los humanos
    gfx_.circle(L(13, 57 + bob), 1.8 * s, rgb(15, 15, 18), 8);

    switch (vr) {
      case 0:   // Pablo Escobar: pelo oscuro rizado y bigote
        gfx_.circle(L(6, 62 + bob), 7.0 * s, rgb(30, 20, 16), 12);
        gfx_.line(L(10, 51 + bob), L(18, 51 + bob), 2.8 * s, rgb(30, 20, 16));
        break;
      case 1:   // Gengis Kan: gorro de piel con punta roja y barba
        gfx_.polygon({L(-2, 60 + bob), L(18, 60 + bob), L(15, 69 + bob), L(1, 69 + bob)}, rgb(60, 40, 30));
        gfx_.polygon({L(4, 69 + bob), L(12, 69 + bob), L(8, 78 + bob)}, rgb(170, 40, 30));
        gfx_.polygon({L(10, 50 + bob), L(18, 50 + bob), L(13, 40 + bob)}, rgb(35, 25, 20));
        break;
      case 2:   // Hitler: peinado de lado y bigote pequeno
        gfx_.polygon({L(-1, 60 + bob), L(16, 63 + bob), L(18, 56 + bob), L(0, 56 + bob)}, rgb(30, 22, 18));
        gfx_.line(L(12, 51 + bob), L(16, 51 + bob), 3.2 * s, rgb(25, 18, 15));
        break;
      case 3:   // Messi: pelo castano y barba corta
        gfx_.circle(L(6, 62 + bob), 6.5 * s, rgb(75, 48, 32), 12);
        gfx_.line(L(6, 48 + bob), L(16, 49 + bob), 4.0 * s, rgb(85, 58, 42), true);
        break;
    }
  }

  void drawBat(const Monster & m)
  {
    const double fd = m.facing;
    auto L = [&](double f, double h) {return Vec2{m.p.x + f * fd, m.p.y - h};};
    Color body = rgb(52, 26, 66);
    Color wing = rgb(78, 36, 92);
    if (m.hurt > 0.0) {
      body = mix(body, rgb(255, 255, 255), 0.7);
      wing = mix(wing, rgb(255, 255, 255), 0.7);
    }
    const double flap = std::sin(m.anim);          // -1..1: alas arriba/abajo
    // ala lejana (mas oscura) y ala cercana
    gfx_.polygon({L(2, 4), L(12, 8 + 16 * flap), L(-4, 12 + 34 * flap), L(-16, 4 + 10 * flap)}, mix(wing, rgb(0, 0, 0), 0.35));
    gfx_.ellipse(m.p, 10.0, 8.0, body, 14);        // cuerpo
    gfx_.circle(L(10, 3), 6.5, body, 12);           // cabeza
    gfx_.polygon({L(8, 8), L(10, 16), L(13, 8)}, body);                // orejas
    gfx_.polygon({L(11, 7), L(15, 14), L(16, 7)}, body);
    gfx_.polygon({L(3, 5), L(18, 6 + 20 * flap), L(-8, 14 + 40 * flap), L(-22, 2 + 14 * flap)}, wing);
    gfx_.glow(L(13, 4), 8.0, rgb(255, 40, 30, 160), rgb(255, 40, 30, 0), 12);
    gfx_.circle(L(13, 4), 2.0, rgb(255, 70, 50), 8);                   // ojo
    gfx_.polygon({L(14, 0), L(15.5, 0), L(15, -4)}, rgb(245, 245, 235));   // colmillo
  }

  // ---- Balones de futbol de Messi ----
  void drawBalls()
  {
    const Vec2 v = gfx_.view();
    for (const Ball & b : balls_) {
      if (b.p.x < v.x - 60.0 || b.p.x > v.x + WIN_W + 60.0) {continue;}
      // Sombra pequeña en el suelo.
      gfx_.ellipse({b.p.x, track_.at(b.p.x) + 1.0}, BALL_R * 0.9, 2.5, rgb(0, 0, 0, 90), 12);
      gfx_.circle(b.p, BALL_R, rgb(250, 250, 250), 18);                 // balon blanco
      // Pentagono central y 5 manchas alrededor que giran con el balon.
      std::vector<Vec2> pent;
      for (int i = 0; i < 5; ++i) {
        const double a = b.spin + 2.0 * PI * i / 5.0;
        pent.push_back(b.p + Vec2{std::cos(a), std::sin(a)} * 4.5);
      }
      gfx_.polygon(pent, rgb(25, 25, 30));
      for (int i = 0; i < 5; ++i) {
        const double a = b.spin + 2.0 * PI * i / 5.0;
        gfx_.circle(b.p + Vec2{std::cos(a), std::sin(a)} * (BALL_R - 2.5), 2.2, rgb(25, 25, 30), 6);
      }
      gfx_.ring(b.p, BALL_R - 0.5, 1.2, rgb(170, 170, 178), 18);        // contorno
    }
  }

  // ---- Luz del faro de la moto ----
  void drawHeadlight()
  {
    const Vec2 lamp = moto_.local(44, 55);
    const Vec2 p1 = moto_.local(44 + 420, 55 + 40);
    const Vec2 p2 = moto_.local(44 + 420, 55 - 120);
    // Triangulo con la punta brillante y los extremos transparentes.
    // Dibujamos tres triangulos pequeños para que el degradado se vea suave.
    std::vector<SDL_Vertex> vs(3);
    const Vec2 pts[3] = {lamp, p1, p2};
    const Color cols[3] = {rgb(255, 240, 170, 120), rgb(255, 240, 170, 0), rgb(255, 240, 170, 0)};
    for (int i = 0; i < 3; ++i) {
      vs[i].position = gfx_.toScreen(pts[i]);
      vs[i].color = cols[i];
      vs[i].tex_coord = SDL_FPoint{0.0f, 0.0f};
    }
    SDL_RenderGeometry(ren_, nullptr, vs.data(), 3, nullptr, 0);
  }

  // ---- Moto XYZ 250 y piloto ----
  void drawWheel(Vec2 c, double spin)
  {
    gfx_.circle(c, WHEEL_R, rgb(20, 20, 22), 28);                     // llanta
    for (int k = 0; k < 14; ++k) {                                    // tacos de la llanta (giran)
      const double a = spin + 2.0 * PI * k / 14.0;
      const Vec2 d{std::cos(a), std::sin(a)};
      gfx_.line(c + d * (WHEEL_R - 4.5), c + d * (WHEEL_R - 0.5), 3.2, rgb(58, 58, 62));
    }
    gfx_.circle(c, WHEEL_R - 6.0, rgb(150, 152, 160), 24);            // rin
    gfx_.circle(c, WHEEL_R - 9.0, rgb(40, 42, 48), 20);
    for (int k = 0; k < 6; ++k) {                                     // rayos (giran)
      const double a = spin + PI * k / 3.0;
      gfx_.line(c, c + Vec2{std::cos(a), std::sin(a)} * (WHEEL_R - 7.0), 1.6, rgb(205, 205, 215));
    }
    gfx_.circle(c, 3.5, rgb(110, 110, 120), 10);                      // centro
  }

  void drawRiderLegs()
  {
    const Vec2 u = riderAxis();
    const Color pants = rgb(38, 48, 80);
    gfx_.line(pose_.hip, pose_.knee, 10.0, pants, true);
    gfx_.line(pose_.knee, pose_.foot, 8.5, pants, true);
    gfx_.line(pose_.foot, pose_.foot + u * 11.0, 7.5, rgb(24, 22, 24), true);   // bota
  }

  void drawRiderUpper()
  {
    const Vec2 u = riderAxis();
    const Vec2 w = riderUp();
    const Pose & p = pose_;
    const Color jacket = rgb(78, 52, 40);          // chamarra de cuero cafe
    const Color jacketDark = rgb(56, 38, 30);
    const Color stripe = rgb(245, 205, 45);        // franjas reflejantes amarillas
    const Color glove = rgb(28, 28, 32);

    // brazo que sujeta el manubrio (va detras del torso)
    gfx_.line(p.shoulder, p.elbowBar, 8.0, jacketDark, true);
    gfx_.line(p.elbowBar, p.handBar, 7.0, jacketDark, true);
    gfx_.circle(p.handBar, 4.0, glove, 10);

    // torso con chamarra
    const Vec2 td = normalized(p.shoulder - p.hip);
    const Vec2 tp{-td.y, td.x};
    gfx_.line(p.hip, p.shoulder, 18.0, jacket, true);
    for (double t : {0.30, 0.55}) {                                        // franjas reflejantes
      const Vec2 c = lerp(p.hip, p.shoulder, t);
      gfx_.line(c + tp * 9.0, c - tp * 9.0, 2.8, stripe);
    }
    gfx_.circle(p.shoulder, 7.0, jacketDark, 12);                          // cuello de la chamarra

    // brazo que sostiene el arma
    gfx_.line(p.shoulder, p.elbowGun, 8.0, jacket, true);
    gfx_.line(p.elbowGun, p.handGun, 7.0, jacket, true);
    gfx_.line(lerp(p.elbowGun, p.handGun, 0.55) + Vec2{0, -3}, lerp(p.elbowGun, p.handGun, 0.55) + Vec2{0, 3}, 2.4, stripe);

    drawGun();
    gfx_.circle(p.handGun, 4.0, glove, 10);

    // casco
    const Vec2 H = p.head;
    gfx_.circle(H, 10.5, rgb(32, 32, 38), 20);                             // casco negro mate
    gfx_.circle(H + u * -2.0 + w * 4.0, 5.0, rgb(70, 70, 80, 140), 12);    // brillo
    gfx_.line(H + u * -9.0 + w * 3.0, H + u * 6.0 + w * 10.0, 2.6, rgb(225, 50, 40));   // franja roja
    gfx_.circle(H + u * 7.0 + w * -6.0, 5.0, rgb(28, 28, 34), 12);        // mentonera
    gfx_.polygon({H + u * 3.0 + w * 6.0, H + u * 10.5 + w * 4.0, H + u * 11.5 + w * -2.0, H + u * 3.0 + w * -1.0}, rgb(255, 170, 40, 240));   // visor ambar
    gfx_.line(H + u * 5.0 + w * 4.5, H + u * 9.5 + w * 3.0, 1.5, rgb(255, 235, 170));  // reflejo del visor
  }

  // Dibuja el arma (un rifle con mira y cargador) apuntando hacia aim_.
  void drawGun()
  {
    const double side = aim_.x >= 0.0 ? 1.0 : -1.0;
    const Vec2 gd{-aim_.y * side, aim_.x * side};      // "abajo" del arma
    const Vec2 base = pose_.handGun;
    // G(f,h): f = hacia adelante del arma, h = hacia arriba del arma.
    auto G = [&](double f, double h) {return base + aim_ * f - gd * h;};

    gfx_.polygon({G(-17, -6), G(-4, -3), G(-4, 3), G(-17, 1)}, rgb(112, 70, 40));              // culata de madera
    gfx_.polygon({G(5, -4), G(12, -4), G(10, -14), G(3, -13)}, rgb(38, 40, 46));               // cargador
    gfx_.polygon({G(-4, -4), G(23, -4), G(23, 3.5), G(-4, 3.5)}, rgb(54, 56, 62));             // cuerpo
    gfx_.polygon({G(23, -1.6), G(40, -1.6), G(40, 1.6), G(23, 1.6)}, rgb(24, 24, 28));         // cañon
    gfx_.polygon({G(6, 3.5), G(19, 3.5), G(19, 7), G(6, 7)}, rgb(30, 32, 38));                 // mira telescopica
    gfx_.circle(G(19, 5.2), 1.8, rgb(90, 190, 255), 8);                                        // lente azul
  }

  void drawMoto()
  {
    const Moto & m = moto_;
    auto L = [&](double f, double h) {return m.local(f, h);};

    const Color orange = rgb(238, 106, 24);       // plasticos de la XYZ 250
    const Color orangeDark = rgb(190, 78, 14);
    const Color black = rgb(24, 24, 28);
    const Color metal = rgb(190, 192, 202);
    const Color engine = rgb(88, 90, 100);

    // Pequeña sombra bajo la moto (solo si esta cerca del suelo).
    {
      const double gy = track_.at(m.mid().x);
      const double dist = gy - m.mid().y;
      if (dist < 140.0) {
        gfx_.ellipse({m.mid().x, gy + 1.0}, 62.0, 4.0, rgb(0, 0, 0, static_cast<int>(110 * (1.0 - dist / 140.0))), 20);
      }
    }

    drawWheel(m.rear.p, m.spin_r);
    drawWheel(m.front.p, m.spin_f);

    // Amortiguador y brazo oscilante (une la rueda trasera al motor).
    gfx_.line(L(-48, 0), L(-8, 14), 6.0, rgb(58, 58, 64), true);
    gfx_.line(L(-30, 38), L(-14, 14), 3.5, rgb(230, 190, 40), true);

    // Escape: tubo + silenciador.
    gfx_.line(L(22, 24), L(16, 6), 4.5, rgb(130, 112, 124), true);
    gfx_.line(L(16, 6), L(-28, 12), 4.5, rgb(130, 112, 124), true);
    gfx_.polygon({L(-62, 12), L(-28, 8), L(-28, 19), L(-62, 18)}, rgb(150, 150, 160));
    gfx_.polygon({L(-65, 12.5), L(-62, 12), L(-62, 18), L(-65, 17.5)}, black);

    // Motor: bloque + cilindro con aletas de enfriamiento.
    gfx_.polygon({L(-16, 4), L(18, 2), L(24, 16), L(18, 27), L(-12, 29), L(-18, 16)}, engine);
    gfx_.polygon({L(4, 26), L(22, 26), L(28, 44), L(10, 44)}, mix(engine, rgb(255, 255, 255), 0.12));
    for (int k = 0; k < 4; ++k) {
      gfx_.line(L(6 + k * 1.4, 29 + k * 4.0), L(24 + k * 1.4, 29 + k * 4.0), 1.4, rgb(50, 52, 60));
    }
    gfx_.circle(L(2, 14), 6.0, rgb(120, 122, 134), 12);        // tapa del motor
    gfx_.circle(L(2, 14), 2.5, rgb(60, 62, 70), 8);

    drawRiderLegs();                                           // la pierna va delante del motor

    // Tanque de gasolina, asiento y cola.
    gfx_.polygon({L(-6, 40), L(8, 52), L(26, 52), L(36, 45), L(31, 35), L(-2, 33)}, orange);
    gfx_.polygon({L(-6, 40), L(8, 52), L(26, 52), L(24, 49), L(8, 49), L(-5, 38)}, mix(orange, rgb(255, 230, 190), 0.35));  // brillo
    gfx_.polygon({L(-58, 47), L(-40, 45), L(-38, 34), L(-52, 38)}, orange);                 // cola trasera
    gfx_.polygon({L(-44, 41), L(-5, 43), L(-5, 34), L(-42, 34)}, black);                    // asiento
    // Letrero "XYZ 250" girado junto con la moto, sobre el tanque.
    gfx_.textOriented("XYZ 250", L(-1, 47.5), m.axis(), m.up() * -1.0, 0.78, rgb(20, 14, 10));
    gfx_.line(L(-56, 28), L(-40, 30), 2.0, orangeDark);                                   // guardabarros trasero

    // Luz trasera (mas fuerte al frenar).
    const bool braking = in_.brake > 0.0 && state_ == State::RUNNING;
    if (braking) {gfx_.glow(L(-60, 42), 22.0, rgb(255, 40, 30, 190), rgb(255, 40, 30, 0), 14);}
    gfx_.circle(L(-59, 42), 3.0, braking ? rgb(255, 80, 60) : rgb(150, 20, 20), 8);

    // Tijera delantera, guardabarros, manubrio y faro.
    gfx_.line(L(48, 0), L(30, 50), 6.0, rgb(52, 52, 58), true);
    gfx_.line(L(44, 12), L(30, 50), 4.0, metal);                                           // barra cromada
    gfx_.line(L(34, 31), L(54, 29), 4.5, orange, true);                                    // guardabarros delantero
    gfx_.line(L(54, 29), L(62, 20), 4.5, orange, true);
    gfx_.line(L(28, 51), L(38, 57), 3.5, black, true);                                     // manubrio
    gfx_.polygon({L(34, 50), L(46, 47), L(47, 62), L(37, 64)}, mix(orange, black, 0.2));   // carcasa del faro
    gfx_.circle(L(44, 55), 6.0, rgb(255, 246, 200), 14);                                   // lente
    gfx_.glow(L(46, 55), 20.0, rgb(255, 240, 170, 160), rgb(255, 240, 170, 0), 14);

    drawRiderUpper();
  }

  // ---- Globo de dialogo del piloto al morir ----
  // Nuestra fuente no tiene acentos, asi que se escribe sin tilde. El caracter '#'
  // es el signo de apertura de exclamacion.
  void drawDeathSpeech()
  {
    if (state_ != State::CRASHED) {return;}
    const std::string txt = "#AH GONORREA!";
    const double sc = 2.0;
    const double a = std::min(1.0, state_timer_ / 0.5);          // se desvanece al final
    const double w = Gfx::textWidth(txt, sc), h = 7.0 * sc;
    const Vec2 c = pose_.head + Vec2{0.0, -48.0};                 // arriba de la cabeza
    const double bx = c.x - w / 2 - 8.0, by = c.y - h / 2 - 6.0;
    const double bw = w + 16.0, bh = h + 12.0;
    const Color ink = fade(rgb(20, 20, 24), a);
    const Color paper = fade(rgb(255, 255, 255), a * 0.95);
    gfx_.rect(bx - 2, by - 2, bw + 4, bh + 4, ink);              // borde
    gfx_.rect(bx, by, bw, bh, paper);                            // globo
    gfx_.polygon({{c.x - 10, by + bh + 2}, {c.x + 10, by + bh + 2}, {c.x, by + bh + 18}}, ink);   // colita (borde)
    gfx_.polygon({{c.x - 7, by + bh}, {c.x + 7, by + bh}, {c.x, by + bh + 13}}, paper);           // colita
    gfx_.textOriented(txt, {bx + 8.0, by + 6.0}, {1, 0}, {0, 1}, sc, ink);
  }

  // ---- Balas, particulas y destello del disparo ----
  void drawBulletsAndFx()
  {
    for (const Bullet & b : bullets_) {
      const Vec2 tail = b.p - normalized(b.v) * 26.0;
      gfx_.glow(b.p, 9.0, rgb(255, 220, 120, 150), rgb(255, 220, 120, 0), 10);
      gfx_.line(tail, b.p, 3.0, rgb(255, 190, 80, 200));
      gfx_.line(b.p - normalized(b.v) * 14.0, b.p, 1.4, rgb(255, 255, 235));   // nucleo brillante
    }
    const Vec2 v = gfx_.view();
    for (const Fx & f : fx_) {
      if (f.p.x < v.x - 40.0 || f.p.x > v.x + WIN_W + 40.0) {continue;}
      const double ratio = std::clamp(f.life / f.maxLife, 0.0, 1.0);
      const double size = f.grow ? f.size * (1.0 + (1.0 - ratio) * 2.5) : f.size * (0.4 + 0.6 * ratio);
      gfx_.circle(f.p, size, fade(f.color, ratio), 8);
    }
    // Destello en la boca del cañon.
    if (muzzle_t_ > 0.0) {
      const Vec2 t = pose_.gunTip;
      gfx_.glow(t, 34.0, rgb(255, 220, 120, 230), rgb(255, 150, 40, 0), 16);
      const double rot = time_ * 40.0;
      std::vector<Vec2> star;
      for (int i = 0; i < 10; ++i) {
        const double a = rot + 2.0 * PI * i / 10.0;
        const double r = (i % 2 == 0) ? 20.0 : 6.0;
        star.push_back(t + Vec2{std::cos(a), std::sin(a)} * r);
      }
      gfx_.polygon(star, rgb(255, 245, 190));
    }
  }

  // ---- Cenizas flotando sobre toda la pantalla ----
  void drawEmbers(Vec2 cam)
  {
    for (int i = 0; i < 80; ++i) {
      const double depth = 0.3 + 0.7 * hash01(i * 3 + 1);
      const double x = wrapTo(hash01(i * 3) * WIN_W - cam.x * depth * 0.6 + std::sin(time_ * 0.8 + i) * 24.0, WIN_W);
      const double y = wrapTo(hash01(i * 3 + 2) * WIN_H + time_ * (18.0 + 40.0 * depth) - cam.y * 0.2, WIN_H);
      const double s = 1.2 + 2.0 * depth;
      const bool hot = hash01(i * 5 + 7) > 0.6;
      gfx_.rect(x, y, s, s, hot ? rgb(255, 150, 50, 190) : rgb(120, 110, 110, 150));
    }
  }

  // ---- Vineta: oscurece los bordes para dar ambiente ----
  void drawVignette()
  {
    const Color dark = rgb(8, 2, 10, 150);
    const Color none = rgb(8, 2, 10, 0);
    gfx_.hGradient(0, 0, 220, WIN_H, dark, none);
    gfx_.hGradient(WIN_W - 220, 0, 220, WIN_H, none, dark);
    gfx_.vGradient(0, 0, WIN_W, 130, dark, none);
    gfx_.vGradient(0, WIN_H - 130, WIN_W, 130, none, dark);
    if (hurt_flash_ > 0.0) {                       // pantalla roja al recibir dano
      gfx_.rect(0, 0, WIN_W, WIN_H, rgb(200, 0, 0, static_cast<int>(110 * hurt_flash_ / 0.35)));
    }
  }

  // ---- HUD: vida, balas, puntos, progreso, mira y mensajes ----
  void drawHud()
  {
    const Color white = rgb(255, 255, 255);

    // Vida
    gfx_.text("VIDA", 20, 14, 2, white);
    gfx_.rect(18, 34, 224, 20, rgb(0, 0, 0, 170));
    const double hp = std::clamp(health_ / MAX_HEALTH, 0.0, 1.0);
    gfx_.rect(20, 36, 220 * hp, 16, mix(rgb(220, 40, 40), rgb(80, 215, 90), hp));
    gfx_.rect(20, 36, 220 * hp, 5, rgb(255, 255, 255, 60));

    // Balas
    gfx_.text("BALAS: " + std::to_string(ammo_), 20, 64, 2, ammo_ > 5 ? white : rgb(255, 90, 70));

    // Velocidad
    const int kmh = static_cast<int>(len(moto_.velocity()) * 0.15);
    gfx_.text("KM/H: " + std::to_string(kmh), 20, WIN_H - 34, 2, white);

    // Puntos y bajas (alineados a la derecha)
    const std::string sc = "PUNTOS: " + std::to_string(score_);
    const std::string kl = "BAJAS: " + std::to_string(kills_);
    gfx_.text(sc, WIN_W - 20 - Gfx::textWidth(sc, 2), 14, 2, rgb(255, 220, 90));
    gfx_.text(kl, WIN_W - 20 - Gfx::textWidth(kl, 2), 38, 2, white);

    // Barra de progreso hacia la meta (con marcas de checkpoint).
    const double bx = WIN_W / 2.0 - 150.0, by = 20.0;
    gfx_.rect(bx - 2, by - 2, 304, 12, rgb(0, 0, 0, 170));
    const double prog = std::clamp(moto_.mid().x / FINISH_X, 0.0, 1.0);
    gfx_.rect(bx, by, 300 * prog, 8, rgb(240, 130, 40));
    for (int i = 0; checkpointX(i) < FINISH_X; ++i) {
      gfx_.rect(bx + 300 * checkpointX(i) / FINISH_X - 1, by - 3, 3, 14, rgb(255, 255, 255, 200));
    }
    gfx_.circle({bx + 300 * prog, by + 4}, 6.0, rgb(255, 235, 120), 12);

    // Ayuda de controles (solo al principio).
    if (hint_time_ > 0.0) {
      const double a = std::min(1.0, hint_time_ / 2.0);
      gfx_.textCentered("W/S: ACELERAR/FRENAR   A/D: INCLINAR", WIN_W / 2.0, WIN_H - 82, 2, fade(white, a));
      gfx_.textCentered("MOUSE: APUNTAR   CLICK/ESPACIO: DISPARAR   R: REINICIAR", WIN_W / 2.0, WIN_H - 56, 2, fade(white, a));
      if (pad_) {
        gfx_.textCentered("CONTROL: RT ACELERAR  B FRENO  A DISPARAR  STICK DER APUNTAR  STICK IZQ INCLINAR", WIN_W / 2.0, WIN_H - 108, 2, fade(white, a));
      }
    }

    // Mensajes cortos (checkpoint, recogiste algo, sin balas).
    if (banner_t_ > 0.0 && state_ == State::RUNNING) {
      gfx_.textCentered(banner_, WIN_W / 2.0, 70, 3, fade(rgb(255, 235, 120), std::min(1.0, banner_t_)));
    }
    if (no_ammo_t_ > 0.0) {gfx_.textCentered("SIN BALAS!", WIN_W / 2.0, 110, 3, rgb(255, 80, 60));}

    // Mensajes grandes de fin.
    if (state_ == State::CRASHED) {
      gfx_.textCentered(death_msg_, WIN_W / 2.0, 190, 7, rgb(235, 50, 40));
      gfx_.textCentered("VOLVIENDO AL CHECKPOINT...", WIN_W / 2.0, 262, 2, white);
    } else if (state_ == State::FINISHED) {
      gfx_.textCentered("META! GANASTE", WIN_W / 2.0, 190, 7, rgb(90, 235, 120));
      gfx_.textCentered("PUNTOS: " + std::to_string(score_) + "   BAJAS: " + std::to_string(kills_), WIN_W / 2.0, 262, 3, white);
    }

    // Mira del arma (donde apuntas con el mouse).
    if (state_ == State::RUNNING && (mouse_seen_ || aimFromPad_)) {
      // Con el control, la mira flota 230 px delante del hombro, en la direccion del stick.
      const Vec2 m = aimFromPad_ ? (moto_.local(-9, 64) - cam_ + aim_ * 230.0) : mouse_;
      const Color c = rgb(255, 235, 140, 230);
      gfx_.ring(m, 11.0, 2.0, c, 20);
      gfx_.rect(m.x - 18, m.y - 1, 10, 2, c);
      gfx_.rect(m.x + 8, m.y - 1, 10, 2, c);
      gfx_.rect(m.x - 1, m.y - 18, 2, 10, c);
      gfx_.rect(m.x - 1, m.y + 8, 2, 10, c);
    }
  }

  // ======================= VARIABLES DEL JUEGO =========================

  SDL_Window * win_{nullptr};
  SDL_Renderer * ren_{nullptr};
  Gfx gfx_{nullptr};

  Track track_;
  Moto moto_;
  Pose pose_;
  Vec2 aim_{1.0, 0.0};
  std::vector<Bullet> bullets_;
  std::vector<Ball> balls_;           // balones que Messi patea
  std::vector<Monster> monsters_;
  std::vector<Spawn> spawns_;
  std::vector<char> spawn_dead_;
  std::vector<Pickup> pickups_;
  std::vector<Fx> fx_;
  std::mt19937 rng_{12345};

  Input in_;
  Vec2 mouse_{0.0, 0.0};
  bool mouse_seen_{false};
  SDL_GameController * pad_{nullptr};   // el control abierto (nullptr = no hay control)
  Vec2 padAim_{1.0, 0.0};               // ultima direccion del stick izquierdo (largo 1)
  bool aimFromPad_{false};              // true = apuntamos con el stick, false = con el mouse

  // Piloto caido (ragdoll), ver startRagdoll().
  Mass rag_[RAG_N];                     // los 9 puntos del cuerpo
  std::vector<RagStick> ragSticks_;     // los huesos que los unen
  double ragBase_{0.0};                 // angulo para orientar casco y bota (ver riderAxis)
  double ragHitCd_{0.0};                // espera entre polvaredas de golpe
  int crash_frames_{0};                 // cuadros desde el choque (para la camara lenta)

  State state_{State::RUNNING};
  double state_timer_{0.0};
  double time_{0.0};
  Vec2 cam_{0.0, 0.0}, shakeOff_{0.0, 0.0};
  double shake_{0.0};

  double health_{MAX_HEALTH};
  int ammo_{START_AMMO};
  int score_{0}, kills_{0};
  double checkpoint_x_{0.0};
  double invuln_{0.0}, hurt_flash_{0.0}, muzzle_t_{0.0}, fire_cd_{0.0};
  double banner_t_{0.0}, no_ammo_t_{0.0}, hint_time_{0.0};
  std::string banner_, death_msg_;

  double remote_throttle_{0.0}, remote_lean_{0.0};
  bool remote_fire_{false};
  std::chrono::steady_clock::time_point last_cmd_{}, last_fire_cmd_{};
};

}  // namespace

// ---------------------------------------------------------------------------
// 9. NODO DE ROS 2 (opcional) Y MAIN
// ---------------------------------------------------------------------------

#ifndef NO_ROS

// Un "nodo" de ROS 2 es un programa que se comunica con otros mediante topics.
// Este nodo solo hace de puente: el juego vive dentro de 'game_'.
class MotoGameNode : public rclcpp::Node
{
public:
  MotoGameNode()
  : Node("bike_game")
  {
    score_pub_ = create_publisher<std_msgs::msg::Int32>("score", 10);
    health_pub_ = create_publisher<std_msgs::msg::Int32>("moto/health", 10);
    pose_pub_ = create_publisher<geometry_msgs::msg::PoseStamped>("bike/pose", 10);

    cmd_sub_ = create_subscription<geometry_msgs::msg::Twist>(
      "bike/cmd_vel", 10, [this](geometry_msgs::msg::Twist::ConstSharedPtr m) {
        game_->setRemoteControl(m->linear.x, m->angular.z);
      });
    fire_sub_ = create_subscription<std_msgs::msg::Bool>(
      "moto/fire", 10, [this](std_msgs::msg::Bool::ConstSharedPtr m) {
        game_->setRemoteFire(m->data);
      });

    game_ = std::make_unique<Game>();

    // Cada 1/60 de segundo se ejecuta un cuadro del juego.
    timer_ = create_wall_timer(std::chrono::microseconds(16667), [this]() {onTick();});
    RCLCPP_INFO(get_logger(), "Moto XYZ 250 lista. WASD mover, mouse apuntar, click disparar.");
  }

private:
  void onTick()
  {
    if (!game_->tick()) {      // el jugador cerro la ventana o pulso ESC
      rclcpp::shutdown();
      return;
    }
    // Publicamos solo cuando cambia el valor (para no saturar la red).
    if (game_->score() != last_score_) {
      last_score_ = game_->score();
      std_msgs::msg::Int32 m;
      m.data = last_score_;
      score_pub_->publish(m);
    }
    if (game_->health() != last_health_) {
      last_health_ = game_->health();
      std_msgs::msg::Int32 m;
      m.data = last_health_;
      health_pub_->publish(m);
    }
    geometry_msgs::msg::PoseStamped p;
    p.header.stamp = now();
    p.header.frame_id = "world";
    const Vec2 pos = game_->motoPos();
    const double yaw = game_->motoYaw();
    p.pose.position.x = pos.x;
    p.pose.position.y = -pos.y;                 // en ROS, y positivo es hacia arriba
    p.pose.orientation.z = std::sin(yaw / 2.0);
    p.pose.orientation.w = std::cos(yaw / 2.0);
    pose_pub_->publish(p);
  }

  std::unique_ptr<Game> game_;
  rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr score_pub_, health_pub_;
  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr pose_pub_;
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_sub_;
  rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr fire_sub_;
  rclcpp::TimerBase::SharedPtr timer_;
  int last_score_{-1}, last_health_{-1};
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  try {
    auto node = std::make_shared<MotoGameNode>();
    rclcpp::spin(node);
  } catch (const std::exception & e) {
    RCLCPP_ERROR(rclcpp::get_logger("bike_game"), "%s", e.what());
  }
  rclcpp::shutdown();
  return 0;
}

#else   // ---------- version SIN ROS: solo SDL ----------

int main()
{
  try {
    Game game;
    // Bucle principal: repetir tick() ~60 veces por segundo hasta que devuelva false.
    while (true) {
      const Uint32 t0 = SDL_GetTicks();
      if (!game.tick()) {break;}
      const Uint32 elapsed = SDL_GetTicks() - t0;
      if (elapsed < 16) {SDL_Delay(16 - elapsed);}     // esperar para no pasar de 60 FPS
    }
  } catch (const std::exception & e) {
    std::fprintf(stderr, "Error: %s\n", e.what());
    return 1;
  }
  return 0;
}

#endif