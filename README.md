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
