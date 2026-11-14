# Información
- **Nombre del Proyecto:** Ledge Out
- **Género / Modo:** Party Brawler / Platform Fighter (Last Man Standing por Vidas)
- **Motor:** Unreal Engine 5.6.1
 
## Esquema del Gameplay
1. **Mecánica Principal:** Juego de combate en tercera persona inspirado en *Super Smash Bros.* y *Gang Beasts*. Los jugadores combaten sobre una plataforma suspendida. Cada golpe recibido incrementa un porcentaje de daño acumulado, aumentando de forma proporcional el retroceso (*knockback*) recibido tras cada impacto.
2. **Ciclo de Partida y Victoria:** 
   - Los jugadores reaparecen sobre la arena al inicio de cada ronda o tras ser eliminados.
   - Ser expulsado fuera de los límites de la plataforma (*Ring Out*) resta 1 de 3 vidas.
   - Gana el último jugador en pie (*Last Man Standing*).
3. **Esquema del HUD / UI:** 
   - **Esquina inferior izquierda (Jugador local):** Contador de vidas restantes e indicador del porcentaje de daño acumulado.
   - **Lateral derecho (Tabla de clasificación en vivo):** Lista de rivales ordenada dinámicamente de mayor a menor según cantidad de vidas y menor daño recibido.
   - **Centro superior (Cronómetro):** Temporizador de la ronda (3 a 5 minutos). Al agotarse el tiempo, se activa una fase de **Muerte Súbita (*Sudden Death*)** para forzar el desempate (ej. daño fijado al máximo o reducción del área de combate).

## Features
- Los jugadores se golpean con Mouse Click Izq.
- Al morir el jugador pierde una vida, no reaparece si se queda sin.
- Cada ataque acumula daño, amentando el knockback y probabilidad de caerse de la arena
- Dos personajes jugables, Aurora, Kwang. Cada uno con Skins selecionables.

## Steam (pruebas)
- El proyecto usa Steam como Online Subsystem y Steam Sockets para el transporte de sesiones y beacons.
- `SteamDevAppId=480` habilita las pruebas con Spacewar; para las invitaciones, ambos jugadores deben ejecutar el juego con Steam iniciado.
- Las invitaciones aceptadas se unen a la sesión Steam y conectan al beacon con la dirección resuelta por Online Subsystem, sin solicitar ni compartir una IP manualmente.
- Para partidas en LAN, activa la opción LAN del menú; las sesiones normales usan Steam Presence.
- La conexión remota no ha sido testeada lo suficiente

## Disclaimer

Este proyecto utiliza modelos, animaciones, texturas y otros recursos correspondientes a los personajes **Aurora** y **Kwang**, pertenecientes al videojuego *Paragon* (propiedad de Epic Games, Inc.), liberados por Epic Games para su uso dentro del ecosistema de Unreal Engine.

El autor de este repositorio **no posee derechos de propiedad intelectual** sobre dichos personajes ni sobre los assets asociados. Este proyecto fue desarrollado exclusivamente con fines educativos, académicos y de demostración técnica, sin ánimo de lucro ni intención de infringir derechos de autor.
