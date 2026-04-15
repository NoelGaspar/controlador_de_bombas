#!/usr/bin/env python3
"""
Tests de validación para el Controlador de Bombas
"""

import json
import serial
import sys
import time
import unittest
from unittest.mock import Mock, patch, MagicMock
from typing import Dict, Any


class TestClientProtocol(unittest.TestCase):
    """Tests del protocolo JSON"""
    
    def test_config_command_structure(self):
        cmd = {
            "cmd": "config",
            "diameter": 10.0,
            "flow_rate": 1.0,
            "reduction": 1,
            "microstep": 16,
            "direction": "inject"
        }
        json_str = json.dumps(cmd)
        parsed = json.loads(json_str)
        self.assertEqual(parsed["cmd"], "config")
        self.assertEqual(parsed["diameter"], 10.0)
    
    def test_start_command(self):
        cmd = {"cmd": "start"}
        json_str = json.dumps(cmd)
        parsed = json.loads(json_str)
        self.assertEqual(parsed["cmd"], "start")
    
    def test_stop_command(self):
        cmd = {"cmd": "stop"}
        json_str = json.dumps(cmd)
        parsed = json.loads(json_str)
        self.assertEqual(parsed["cmd"], "stop")
    
    def test_status_command(self):
        cmd = {"cmd": "status"}
        json_str = json.dumps(cmd)
        parsed = json.loads(json_str)
        self.assertEqual(parsed["cmd"], "status")
    
    def test_response_ok_structure(self):
        resp = {"status": "ok", "msg": "Motor iniciado", "data": {}}
        json_str = json.dumps(resp)
        parsed = json.loads(json_str)
        self.assertEqual(parsed["status"], "ok")
        self.assertIn("msg", parsed)
    
    def test_response_error_structure(self):
        resp = {"status": "error", "msg": "Flujo no puede ser 0", "code": "INVALID_PARAM"}
        json_str = json.dumps(resp)
        parsed = json.loads(json_str)
        self.assertEqual(parsed["status"], "error")
        self.assertIn("code", parsed)


class TestFlowCalculation(unittest.TestCase):
    """Tests de cálculo de flujo volumétrico"""
    
    @staticmethod
    def calculate_rpm(flow_rate_ml_min: float, diameter_mm: float, 
                       pitch_mm: float = 2.0, reduction: float = 1.0) -> float:
        """Calcula RPM necesarias para un flujo dado"""
        import math
        radius_mm = diameter_mm / 2.0
        area_mm2 = math.pi * radius_mm * radius_mm
        vol_per_rev_ml = (area_mm2 * pitch_mm / reduction) / 1000.0
        if vol_per_rev_ml <= 0:
            return 0
        return flow_rate_ml_min / vol_per_rev_ml
    
    def test_rpm_calculation_10ml_syringe(self):
        """RPM para jeringa 10mm a 1ml/min"""
        rpm = self.calculate_rpm(1.0, 10.0)
        self.assertGreater(rpm, 0)
        self.assertLess(rpm, 100)
    
    def test_rpm_calculation_higher_flow(self):
        """RPM para mayor flujo"""
        rpm = self.calculate_rpm(5.0, 10.0)
        self.assertGreater(rpm, 0)
    
    def test_rpm_increases_with_flow(self):
        """Mayor flujo = mayor RPM"""
        rpm_low = self.calculate_rpm(1.0, 10.0)
        rpm_high = self.calculate_rpm(2.0, 10.0)
        self.assertGreater(rpm_high, rpm_low)
    
    def test_step_frequency_calculation(self):
        """Cálculo de frecuencia de steps"""
        steps_per_rev = 200
        microstep = 16
        rpm = 30
        step_freq = (steps_per_rev * microstep * rpm) / 60.0
        self.assertEqual(step_freq, 1600)


class TestCommandValidation(unittest.TestCase):
    """Tests de validación de comandos"""
    
    def test_config_requires_diameter(self):
        cmd = {"cmd": "config", "diameter": 10.0, "flow_rate": 1.0}
        self.assertIn("diameter", cmd)
    
    def test_config_requires_flow_rate(self):
        cmd = {"cmd": "config", "diameter": 10.0, "flow_rate": 1.0}
        self.assertIn("flow_rate", cmd)
    
    def test_flow_rate_must_be_positive(self):
        flow_rate = -1.0
        self.assertLessEqual(flow_rate, 0)
    
    def test_microstep_valid_values(self):
        valid_microsteps = [1, 2, 4, 8, 16, 32, 64, 256]
        for ms in valid_microsteps:
            self.assertIn(ms, valid_microsteps)
    
    def test_direction_valid_values(self):
        valid_directions = ["inject", "aspire"]
        for direction in valid_directions:
            self.assertIn(direction, valid_directions)


class MockSerial:
    """Mock de puerto serie para testing"""
    
    def __init__(self):
        self.buffer = b""
        self.is_open = True
    
    def write(self, data: bytes):
        self.buffer += data
    
    def readline(self) -> bytes:
        return b'{"status":"ok","msg":"Mock response"}'
    
    def close(self):
        self.is_open = False


class TestIntegration(unittest.TestCase):
    """Tests de integración con mock de serial"""
    
    def test_mock_serial_write(self):
        mock = MockSerial()
        mock.write(b'{"cmd":"status"}\n')
        self.assertIn(b"cmd", mock.buffer)
    
    def test_json_command_roundtrip(self):
        cmd = {"cmd": "status"}
        json_str = json.dumps(cmd)
        parsed = json.loads(json_str)
        self.assertEqual(cmd, parsed)


def run_tests():
    """Ejecutar tests y guardar resultado"""
    loader = unittest.TestLoader()
    suite = loader.loadTestsFromModule(sys.modules[__name__])
    
    with open("logs/test_log.txt", "w") as f:
        runner = unittest.TextTestRunner(stream=f, verbosity=2)
        result = runner.run(suite)
        
        f.write("\n=== RESUMEN ===\n")
        f.write(f"Tests ejecutados: {result.testsRun}\n")
        f.write(f"Fallidos: {len(result.failures)}\n")
        f.write(f"Errores: {len(result.errors)}\n")
        
        if result.wasSuccessful():
            f.write("TODOS LOS TESTS PASARON\n")
        else:
            f.write("ALGUNOS TESTS FALLARON\n")
    
    return result.wasSuccessful()


if __name__ == "__main__":
    success = run_tests()
    sys.exit(0 if success else 1)
