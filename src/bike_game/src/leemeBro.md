# Bike Game
# primero: 


Instrucciones para clonar y configurar el repositorio localmente.

## Instalación

1. Abre tu gestor de archivos y ve a la carpeta **Home**.
2. Haz clic derecho en un espacio vacío y selecciona **Abrir en la terminal**.
3. Inicializa Git (si es necesario) y clona el proyecto con los siguientes comandos:

```bash
git init
git clone https://github.com/santiagocuellarandrade/Bike.git 
```

# Segundo:

utliza estos coamndos:
 source ~/Bike/install/setup.bash // para refrescar el termianal
 colcon build --packages-select bike_game // Para cargar lo nuevos cambios que hagas
 ros2 run bike_game bike_game // Para arrancar el juego en ros2

# tercero:

Instala esto en tu terminal
 notebook:~/Bike$ sudo apt update && sudo apt install -y libsdl2-net-dev


# 🏍️ MOTO XYZ 250 - APOCALIPSIS

¡Un juego de motos con rampas, monstruos y un arma! Y lo mejor: **puedes jugar con un amigo que esté en otro computador**.

Esta guía te explica TODO paso a paso, como si fuera una receta de cocina. No necesitas saber nada de programación. 🙂

---

## 📖 Palabras raras (y qué significan)

Antes de empezar, aquí tienes un mini diccionario. Si ves una palabra rara más abajo, vuelve aquí.

| Palabra | Qué significa (fácil) |
|---|---|
| **Terminal** | Una ventana negra donde le escribes órdenes al computador con el teclado. Es como mandarle mensajes de texto a tu compu. |
| **Compilar** | Convertir el código del juego (que parece un idioma raro) en un programa que el computador sí puede abrir. Es como hornear una torta: antes tienes ingredientes, después tienes la torta lista. |
| **Anfitrión (host)** | El amigo que "abre la sala de juego". Es como el dueño de la casa en una fiesta. |
| **Invitado** | El amigo que entra a la sala del anfitrión. |
| **Dirección IP** | Un número que es como la dirección de tu casa, pero para tu computador. Sirve para que tu amigo sepa dónde encontrarte. Se ve así: `192.168.1.20` |
| **Wifi / red** | La conexión a internet de tu casa. Si dos computadores están en el mismo wifi, están "en la misma red". |
| **Firewall** | Un guardia de seguridad del computador. A veces no deja entrar a nadie, ni siquiera a tu amigo. Hay que decirle que sí puede pasar. |
| **Puerto** | Una puertita del computador. Nuestro juego usa la puertita número `7777`. |

---

## 🧰 Lo que necesitas

- ✅ Dos computadores con **Ubuntu** (un tipo de sistema, como Windows pero gratis). **Los dos** deben tener Ubuntu.
- ✅ El archivo del juego, que se llama **`bike_game.cpp`**
- ✅ Internet
- ✅ Un amigo 😎

> 💡 **¿Tienes Windows o Mac?** Esta guía es para Ubuntu. Los pasos cambian mucho en otros sistemas. Pídele ayuda a quien te pasó esta guía.

---

## 🎮 PARTE 1: Preparar el juego (¡los DOS amigos hacen esto!)

### Paso 1: Abrir la terminal

Presiona estas 3 teclas al mismo tiempo: **Ctrl + Alt + T**

Se abre una ventana (normalmente negra o morada). ¡Esa es la terminal!

🔎 **¿No te funciona?** Busca en internet:
👉 [cómo abrir la terminal en Ubuntu](https://www.google.com/search?q=como+abrir+la+terminal+en+ubuntu)

### Paso 2: Instalar las herramientas

Estas herramientas son como los "ingredientes" que el juego necesita para funcionar.

Copia esta línea, pégala en la terminal y presiona **Enter**:

```
sudo apt install g++ libsdl2-dev libsdl2-net-dev
```

- Te va a pedir tu **contraseña** (la misma con la que prendes el computador).
- Cuando la escribas, **no vas a ver nada en la pantalla**. No es que esté fallando: es una medida de seguridad. Escríbela completa y presiona **Enter**.
- Si te pregunta `¿Desea continuar? [S/n]`, escribe **S** y presiona Enter.
- Espera a que termine. Puede tardar unos minutos. ☕

> 💡 **Truco para pegar en la terminal:** en la terminal NO se pega con Ctrl+V. Se pega con **Ctrl + Shift + V**.

🔎 **¿Salió un error?** Busca en internet:
👉 [sudo apt install no funciona ubuntu](https://www.google.com/search?q=sudo+apt+install+no+funciona+ubuntu)

### Paso 3: Guardar el juego

1. Guarda el código del juego en un archivo llamado exactamente: **`moto_game.cpp`**
2. Anota en qué carpeta lo guardaste (por ejemplo `Descargas` o `Escritorio`).
3. ⚠️ **Los dos amigos deben tener el MISMO archivo**, sin cambiarle nada. Si uno tiene una versión distinta, el juego online puede fallar.

### Paso 4: Entrar a la carpeta donde está el juego

En la terminal, escribe `cd` (que significa "ir a") y el nombre de la carpeta.

Por ejemplo, si lo guardaste en **Descargas**:

```
cd ~/Descargas
```

Si lo guardaste en el **Escritorio**:

```
cd ~/Escritorio
```

> 💡 Para comprobar que estás en el lugar correcto, escribe `ls` y presiona Enter. Debe aparecer `moto_game.cpp` en la lista.

🔎 **¿Estás perdido?** Busca:
👉 [cómo usar el comando cd en la terminal de Linux](https://www.google.com/search?q=como+usar+el+comando+cd+terminal+linux)

### Paso 5: Compilar (hornear la torta 🎂)

Copia y pega esta línea larga y presiona **Enter**:

```
g++ -std=c++17 -O2 -DNO_ROS moto_game.cpp -o moto_game $(sdl2-config --cflags --libs) -lSDL2_net
```

- Va a tardar unos segundos o un minuto. **Es normal que se demore.**
- Si termina y **no escribe nada raro en rojo**, ¡todo salió bien! ✅
- Ahora debe existir un programa nuevo llamado **`moto_game`**.

**¿Algo salió mal?**

| Si ves esto... | Haz esto |
|---|---|
| `sdl2-config: command not found` | Repite el **Paso 2** (faltó instalar algo) |
| `SDL_net.h: No such file` | Repite el **Paso 2** |
| `No such file or directory: moto_game.cpp` | No estás en la carpeta correcta. Repite el **Paso 4** |
| Otro error largo en rojo | Copia el mensaje y pégalo en [Claude](https://claude.ai) o en Google para pedir ayuda |

### Paso 6: Probar que el juego abre (solo)

Escribe:

```
./moto_game
```

Debe abrirse una ventana con tu moto. 🏍️ ¡Si abre, vas muy bien! Para cerrarlo presiona **ESC**.

---

## 🌐 PARTE 2: Jugar con tu amigo

Primero decidan quién es el **anfitrión** (el que abre la sala) y quién es el **invitado** (el que entra).

Ahora elige tu caso:

- 🏠 **Caso A:** están en la misma casa, con el mismo wifi
- 🌎 **Caso B:** están en casas diferentes

---

### 🏠 CASO A: Misma casa (mismo wifi)

#### 👑 El anfitrión hace esto:

**1.** Averigua su "dirección". En la terminal escribe:

```
hostname -I
```

Va a salir un número parecido a este: `192.168.1.20`. **¡Ese es tu número!** Anótalo y pásaselo a tu amigo (por WhatsApp, por ejemplo).

**2.** Abre el juego como anfitrión:

```
./moto_game host
```

Verás el mensaje **"ESPERANDO JUGADOR 2..."** en la pantalla. ¡Perfecto, ahora hay que esperar al amigo!

#### 🙋 El invitado hace esto (DESPUÉS de que el anfitrión abrió el juego):

Escribe esto, cambiando el número por el que le dio el anfitrión:

```
./moto_game join 192.168.1.20
```

---

### 🌎 CASO B: Casas diferentes (por internet)

Aquí necesitamos un programa mágico y gratis llamado **Tailscale**. Hace que los dos computadores crean que están en la misma casa, aunque estén muy lejos.

> 🧙 **Cómo funciona la magia:** es como un túnel secreto de internet que une los dos computadores.

#### Paso B1: Instalar Tailscale (LOS DOS amigos)

1. Entren a 👉 **[tailscale.com/download](https://tailscale.com/download)**
2. Elijan **Linux** y copien el comando que aparece en la página.
3. Péguenlo en la terminal (**Ctrl + Shift + V**) y presionen **Enter**.
4. Luego escriban:

```
sudo tailscale up
```

5. Les va a salir un **enlace** en la terminal. Ábranlo en el navegador e **inicien sesión** (pueden usar su cuenta de Google).

🔎 **¿Quieres ver cómo se hace?** Busca videos:
👉 [cómo instalar Tailscale en Ubuntu](https://www.google.com/search?q=como+instalar+tailscale+en+ubuntu)

#### Paso B2: Que los dos estén en la misma "red mágica"

Hay dos formas fáciles:

- **Forma 1 (la más fácil):** los dos inician sesión con **la misma cuenta**.
- **Forma 2:** cada uno usa su cuenta, y el anfitrión **comparte su computador** con el invitado desde 👉 [login.tailscale.com/admin/machines](https://login.tailscale.com/admin/machines).

🔎 Si te pierdes con esto, busca:
👉 [cómo compartir un dispositivo en Tailscale](https://www.google.com/search?q=como+compartir+un+dispositivo+en+tailscale)

#### Paso B3: Averiguar la dirección mágica (el anfitrión)

En la terminal escribe:

```
tailscale ip
```

Va a salir un número que **empieza por 100.**, por ejemplo: `100.64.12.5`. Pásaselo a tu amigo.

#### Paso B4: ¡A jugar!

**👑 El anfitrión:**

```
./moto_game host
```

**🙋 El invitado (después de que el anfitrión abrió el juego):**

```
./moto_game join 100.64.12.5
```

(cambia el número por el que te dio tu amigo)

---

## ✅ ¿Cómo sé que funcionó?

- En la pantalla del anfitrión, primero dice **"ESPERANDO JUGADOR 2..."** (en naranja).
- Cuando el invitado entra, cambia a **"JUGADOR 2: CONECTADO"** (en verde). 🎉
- Vas a ver **otra moto** con un letrero que dice **"JUGADOR 2"**.

> ⚠️ **Muy importante:** el **anfitrión SIEMPRE abre el juego PRIMERO**, y el invitado después. Si lo hacen al revés, no se conectan.

---

## 😟 Si algo no funciona

### Se queda en "ESPERANDO JUGADOR 2..." para siempre

Revisa estas cosas, en orden:

1. ¿El anfitrión abrió el juego **primero**? Si no, cierren los dos juegos (tecla **ESC**) y háganlo en orden.
2. ¿El invitado escribió **bien el número**? Un solo número equivocado y no funciona.
3. ¿Están los dos en **la misma red** (Caso A) o los dos con **Tailscale prendido** (Caso B)?
4. Revisa el **firewall** (el guardia de seguridad). Mira la siguiente sección. 👇

### 🛡️ Decirle al firewall que deje pasar a tu amigo

**Solo el anfitrión** hace esto. En la terminal escribe:

```
sudo ufw allow 7777/udp
```

Esto le dice al guardia: "deja pasar a quien venga por la puertita 7777".

🔎 Más información:
👉 [qué es un firewall explicado fácil](https://www.google.com/search?q=que+es+un+firewall+explicado+facil)

### Otros problemas

| Problema | Qué hacer |
|---|---|
| El juego no abre | Repite el Paso 5 y fíjate si hubo errores |
| Los monstruos son distintos en cada compu | **Es normal.** Cada computador tiene sus propios monstruos (ver más abajo) |
| Hay mucho retraso (lag) | Cierren programas que usen mucho internet (videos, descargas) |
| Quiero probar yo solo | Abre dos terminales: en una `./moto_game host` y en la otra `./moto_game join 127.0.0.1` |

---

## 🧟 Cosas que debes saber del modo online

- Los dos **se ven en tiempo real**: motos, disparos y caídas. ✅
- Pero **cada computador tiene sus propios monstruos**. Si tú matas a un monstruo, en el computador de tu amigo sigue vivo. Cada uno cuenta sus propios puntos.
- Si los dos juegan en **el mismo computador** para probar, el teclado y el mouse controlan las dos ventanas a la vez. Usa un **control de Xbox** en una de ellas.

---

## 🕹️ Controles

### ⌨️ Teclado y mouse

| Tecla | Qué hace |
|---|---|
| **W** o **↑** | Acelerar |
| **S** o **↓** | Frenar |
| **D** o **→** | Inclinarte hacia adelante |
| **A** o **←** | Inclinarte hacia atrás (¡caballito!) |
| **Mouse** | Apuntar el arma |
| **Click izquierdo** o **Espacio** | Disparar |
| **R** | Reiniciar el juego |
| **F12** | Sacar una foto de la pantalla (`captura.bmp`) |
| **ESC** | Salir |

### 🎮 Control de Xbox 360

| Botón | Qué hace |
|---|---|
| **Gatillo derecho (RT)** | Acelerar (mientras más aprietas, más acelera) |
| **Botón A** | Disparar |
| **Botón B** | Frenar |
| **Joystick derecho** | Apuntar el arma |
| **Joystick izquierdo (izquierda/derecha)** | Inclinar la moto |
| **START** | Reiniciar |

> 💡 Conecta el control **antes** de abrir el juego. Si lo detecta, verás un texto extra de ayuda en la pantalla al empezar.

---

## 🎯 Cómo se juega

- Llega a la **META** (un arco de cuadros blancos y negros) sin caerte. 🏁
- Si la **cabeza o la cadera** de tu piloto tocan el suelo, te caes.
- Hay **monstruos** que te atacan: personajes armados, murciélagos y una Estrella Pop gigante.
- Recoge **cajas amarillas** (balas) y **botiquines con cruz roja** (vida).
- Las **banderas** son checkpoints: si mueres, vuelves a la última que pasaste.

---

## 🔗 Enlaces útiles

| Para qué | Enlace |
|---|---|
| Descargar Tailscale (juego por internet) | [tailscale.com/download](https://tailscale.com/download) |
| Administrar tus computadores en Tailscale | [login.tailscale.com/admin/machines](https://login.tailscale.com/admin/machines) |
| Qué es Ubuntu | [ubuntu.com](https://ubuntu.com) |
| La librería gráfica que usa el juego (SDL2) | [libsdl.org](https://www.libsdl.org) |
| Pedir ayuda si algo falla | [claude.ai](https://claude.ai) |

---

## 🆘 Cuando pidas ayuda en internet

Si algo falla y quieres buscar la solución, estos trucos ayudan mucho:

1. **Copia el mensaje de error exacto** que sale en la terminal y pégalo en Google.
2. Agrega la palabra **ubuntu** al final de tu búsqueda.
3. Si le escribes a alguien (o a una inteligencia artificial), cuéntale:
   - Qué paso estabas haciendo
   - Qué mensaje salió
   - Si eres el anfitrión o el invitado

¡Diviértete y que gane el mejor piloto! 🏆🏍️💥
 