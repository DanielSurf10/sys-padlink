import pygame
import time

pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    print("Nenhum joystick detectado!")
    exit()

joy = pygame.joystick.Joystick(0)
joy.init()

print(f"Joystick: {joy.get_name()}")
print(f"Botões: {joy.get_numbuttons()}")
print(f"Eixos: {joy.get_numaxes()}")
print(f"Hats: {joy.get_numhats()}")
print("\nAperte os botões do controle (Ctrl-C para sair)...\n")

button_states = [False] * joy.get_numbuttons()

try:
    while True:
        pygame.event.pump()

        # Verifica botões
        # for i in range(joy.get_numbuttons()):
        #     if joy.get_button(i) and not button_states[i]:
        #         print(f"Botão {i} PRESSIONADO")
        #         button_states[i] = True
        #     # elif not joy.get_button(i) and button_states[i]:
        #     #     print(f"Botão {i} SOLTO")
        #     #     button_states[i] = False

        # # Verifica eixos analógicos
        # for i in range(joy.get_numaxes()):
        #     axis_value = round(joy.get_axis(i), 2)
        #     if abs(axis_value) > 0.5:
        #         print(f"Eixo {i}: {axis_value}")

        # Verifica hat (D-Pad)
        for i in range(joy.get_numhats()):
            hat_value = joy.get_hat(i)
            if hat_value != (0, 0):
                print(f"Hat {i}: {hat_value}")

        time.sleep(0.05)

except KeyboardInterrupt:
    print("\nEncerrando...")
finally:
    pygame.quit()
