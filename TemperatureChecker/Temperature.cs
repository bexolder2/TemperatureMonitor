using System;
using System.Collections.Generic;
using OpenHardwareMonitor.Hardware;

namespace TemperatureChecker
{
    public class Temperature : IDisposable
    {
        private readonly Computer _computer;

        public Temperature()
        {
            _computer = new Computer { CPUEnabled = true };
            _computer.Open();
        }

        public IReadOnlyDictionary<string, float> GetTemperaturesInCelsius()
        {
            var coreAndTemperature = new Dictionary<string, float>();

            foreach (var hardware in _computer.Hardware)
            {
                hardware.Update(); //use hardware.Name to get CPU model
                foreach (var sensor in hardware.Sensors)
                {
                    if (sensor.SensorType == SensorType.Temperature && sensor.Value.HasValue)
                        coreAndTemperature.Add(sensor.Name, sensor.Value.Value);
                }
            }

            return coreAndTemperature;
        }

        public IReadOnlyDictionary<string, float> GetLoadInPercents()
        {
            var coreAndLoad = new Dictionary<string, float>();

            foreach (var hardware in _computer.Hardware)
            {
                hardware.Update();
                foreach (var sensor in hardware.Sensors)
                {
                    if (sensor.SensorType == SensorType.Load && sensor.Value.HasValue)
                        coreAndLoad.Add(sensor.Name, sensor.Value.Value);
                }
            }

            return coreAndLoad;
        }

        public void Dispose()
        {
            try
            {
                _computer.Close();
            }
            catch (Exception)
            {
                //ignore closing errors
            }
        }
    }
}
