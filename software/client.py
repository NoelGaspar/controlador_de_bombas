#!/usr/bin/env python3
"""
Cliente CLI interactivo para el Controlador de Bombas
Comunicación UART via JSON
"""

import argparse
import json
import serial
import sys
import time
from typing import Optional, Dict, Any


class PumpController:
    def __init__(self, port: str, baudrate: int = 115200):
        self.port = port
        self.baudrate = baudrate
        self.serial: Optional[serial.Serial] = None
        self.verbose = False
    
    def connect(self) -> bool:
        try:
            self.serial = serial.Serial(
                port=self.port,
                baudrate=self.baudrate,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=2.0
            )
            time.sleep(0.5)
            return True
        except Exception as e:
            print(f"Error conectando: {e}")
            return False
    
    def disconnect(self):
        if self.serial and self.serial.is_open:
            self.serial.close()
    
    def send_command(self, cmd: Dict[str, Any]) -> Dict[str, Any]:
        if not self.serial or not self.serial.is_open:
            return {"status": "error", "msg": "No conectado", "code": "NOT_CONNECTED"}
        
        try:
            cmd_str = json.dumps(cmd)
            if self.verbose:
                print(f"> {cmd_str}")
            self.serial.write(cmd_str.encode() + b'\n')
            time.sleep(0.2)
            
            # Leer múltiples líneas hasta encontrar JSON válido
            while self.serial.in_waiting:
                line = self.serial.readline()
                if not line:
                    break
                line_str = line.decode('utf-8', errors='replace').strip()
                
                # Ignorar mensajes de debug
                if line_str.startswith('[') and ']' in line_str:
                    if self.verbose:
                        print(f"  DEBUG: {line_str}")
                    continue
                
                # Buscar línea que sea JSON válido
                if line_str.startswith('{'):
                    if self.verbose:
                        print(f"< {line_str}")
                    return json.loads(line_str)
            
            return {"status": "error", "msg": "Sin respuesta", "code": "NO_RESPONSE"}
        except Exception as e:
            return {"status": "error", "msg": str(e), "code": "COMM_ERROR"}
    
    def config(self, diameter: float = 10.0, flow_rate: float = 1.0, 
               reduction: float = 1.0, microstep: int = 16, 
               direction: str = "inject", address: int = 0) -> Dict[str, Any]:
        cmd = {
            "cmd": "config",
            "diameter": diameter,
            "flow_rate": flow_rate,
            "reduction": reduction,
            "microstep": microstep,
            "direction": direction,
            "address": address
        }
        return self.send_command(cmd)
    
    def start(self, duration: float = 0) -> Dict[str, Any]:
        result = self.send_command({"cmd": "start"})
        print(f"Start response: {json.dumps(result, indent=2)}")
        if duration > 0 and result.get("status") == "ok":
            print(f"Running for {duration} seconds...")
            time.sleep(duration)
            stop_result = self.stop()
            print(f"Stop response: {json.dumps(stop_result, indent=2)}")
            return stop_result
        return result
    
    def stop(self) -> Dict[str, Any]:
        return self.send_command({"cmd": "stop"})
    
    def status(self) -> Dict[str, Any]:
        return self.send_command({"cmd": "status"})
    
    def calibrate(self) -> Dict[str, Any]:
        return self.send_command({"cmd": "calibrate"})
    
    def reset(self) -> Dict[str, Any]:
        return self.send_command({"cmd": "reset"})


def test_microstepping(controller: PumpController, microsteps: list):
    """Prueba diferentes valores de microstepping"""
    print("\n=== Prueba de Microstepping ===")
    results = []
    
    for ms in microsteps:
        print(f"\nMicrostepping: {ms}")
        
        result = controller.config(microstep=ms, flow_rate=5.0)
        print(f"  Config: {result.get('status')}")
        
        result = controller.start()
        print(f"  Start: {result.get('status')}")
        
        time.sleep(2)
        
        result = controller.stop()
        print(f"  Stop: {result.get('status')}")
        
        results.append({"microstep": ms, "result": result.get("status")})
        time.sleep(0.5)
    
    return results


def test_flow_rates(controller: PumpController, flows: list):
    """Prueba diferentes flujos"""
    print("\n=== Prueba de Flujos ===")
    results = []
    
    for flow in flows:
        print(f"\nFlujo: {flow} ml/min")
        
        result = controller.config(flow_rate=flow)
        print(f"  Config: {result.get('status')}")
        
        result = controller.start()
        print(f"  Start: {result.get('status')}")
        
        time.sleep(1.5)
        
        result = controller.stop()
        print(f"  Stop: {result.get('status')}")
        
        results.append({"flow": flow, "result": result.get("status")})
        time.sleep(0.5)
    
    return results


def interactive_mode(controller: PumpController):
    """Modo interactivo"""
    print("\n=== Modo Interactivo ===")
    print("Comandos disponibles:")
    print("  status              - Ver estado actual")
    print("  config <params>    - Configurar motor")
    print("  start [segundos]   - Iniciar motor (opcional: duración)")
    print("  stop               - Detener motor")
    print("  test microstep     - Probar diferentes microsteppings")
    print("  test flow          - Probar diferentes flujos")
    print("  quit               - Salir")
    print()
    
    while True:
        try:
            cmd = input("> ").strip().lower()
            
            if not cmd:
                continue
                
            if cmd == "quit" or cmd == "exit":
                break
            
            elif cmd == "status":
                result = controller.status()
                print(json.dumps(result, indent=2))
            
            elif cmd == "start":
                result = controller.start()
                print(json.dumps(result, indent=2))
            
            elif cmd.startswith("start "):
                try:
                    duration = float(cmd.split()[1])
                    result = controller.start(duration=duration)
                    print(json.dumps(result, indent=2))
                except:
                    print("Uso: start [segundos]")
            
            elif cmd == "stop":
                result = controller.stop()
                print(json.dumps(result, indent=2))
            
            elif cmd.startswith("config"):
                parts = cmd.split()
                params = {"microstep": 16, "flow_rate": 1.0, "direction": "inject"}
                
                for i, part in enumerate(parts[1:], 1):
                    if part.isdigit():
                        params["microstep"] = int(part) if i == 1 else params.get("microstep", 16)
                
                result = controller.config(**params)
                print(json.dumps(result, indent=2))
            
            elif cmd == "test microstep":
                test_microstepping(controller, [1, 2, 4, 8, 16])
            
            elif cmd == "test flow":
                test_flow_rates(controller, [0.5, 1.0, 2.0, 5.0])
            
            elif cmd == "help":
                print("Comandos: status, start, stop, config, test microstep, test flow, quit")
            
            else:
                print(f"Comando desconocido: {cmd}")
        
        except KeyboardInterrupt:
            print("\nSaliendo...")
            break
        except Exception as e:
            print(f"Error: {e}")


def main():
    parser = argparse.ArgumentParser(description="Cliente CLI Controlador de Bombas")
    parser.add_argument("-p", "--port", default="/dev/ttyUSB0", help="Puerto serie")
    parser.add_argument("-b", "--baudrate", type=int, default=115200, help="Baudrate")
    parser.add_argument("-v", "--verbose", action="store_true", help="Modo detallado")
    parser.add_argument("-i", "--interactive", action="store_true", help="Modo interactivo")
    parser.add_argument("--diameter", type=float, help="Diámetro jeringa (mm)")
    parser.add_argument("--flow", type=float, help="Flujo (ml/min)")
    parser.add_argument("--microstep", type=int, help="Microstepping")
    parser.add_argument("--direction", choices=["inject", "aspire"], help="Dirección")
    parser.add_argument("--address", type=int, default=0, help="Dirección driver")
    parser.add_argument("--duration", type=float, default=0, help="Duración start (segundos)")
    parser.add_argument("command", nargs="?", choices=["start", "stop", "status", "calibrate", "reset", "config"], 
                        help="Comando a ejecutar")
    
    args = parser.parse_args()
    
    controller = PumpController(args.port, args.baudrate)
    controller.verbose = args.verbose
    
    if not controller.connect():
        print("No se pudo conectar al dispositivo")
        sys.exit(1)
    
    try:
        if args.interactive:
            interactive_mode(controller)
        
        elif args.command == "status":
            result = controller.status()
            print(json.dumps(result, indent=2))
        
        elif args.command == "start":
            result = controller.start(duration=args.duration)
            print(json.dumps(result, indent=2))
        
        elif args.command == "stop":
            result = controller.stop()
            print(json.dumps(result, indent=2))
        
        elif args.command == "config":
            result = controller.config(
                diameter=args.diameter or 10.0,
                flow_rate=args.flow or 1.0,
                microstep=args.microstep or 16,
                direction=args.direction or "inject",
                address=args.address
            )
            print(json.dumps(result, indent=2))
        
        elif args.command == "calibrate":
            result = controller.calibrate()
            print(json.dumps(result, indent=2))
        
        elif args.command == "reset":
            result = controller.reset()
            print(json.dumps(result, indent=2))
        
        else:
            parser.print_help()
    
    finally:
        controller.disconnect()


if __name__ == "__main__":
    main()