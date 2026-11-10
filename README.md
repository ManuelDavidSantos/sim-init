# simnit

A simple init scheme for UNIX based operating systems

### Manual de Uso

El sistema actual está configurado para funcionar de dos formas distintas, usando el supervisor junto al init o solamente operando con el init y dejando la supervisión a discreción del administrador. Es recomendable emparejar SimpleInit con Supervised para aprovechar la máxima funcionalidad del esquema y es la configuración esperada por el manual.

#### Archivos necesarios

SimpleInit y Supervised consisten de un solo binario cada uno, ambos residen en `/bin`, además el binario de SimpleInit debe ser apuntado en la configuración del sistema como PID 1. Los archivos de configuración se almacenan en `/etc/sim-init/`, estos consisten de `r0`, `basics` y `shutdown`. Los archivos `basics` y `shutdown` deben ser ejecutables, en el ejemplo son scripts en bash, `basics` realiza la configuración inicial y todas las tareas _"básicas"_ para el funcionamiento del sistema, en cambio `shutdown` prepara al sistema para apagarse antes de que SimpleInit llame al kernel a apagar el equipo. Los runlevels del sistema se configurarán en archivos `rN`, donde N es el nivel. Actualmente solo hay soporte para un único runlevel por lo que solo hace falta tener `r0` en el directorio. Finalmente algunas utilidades del sistema se almacenaran también en `/bin/`. Entonces la estructura del sistema de archivos se verá de este modo:

```
/bin/
|   sim-init
|   supervised
|   power-off
...
/etc/
|   sim-init/
|   |       r0
|   |       basics
|   |       shutdown
```
El archivo `r0` se configura con el siguiente formato:

La sintaxis del archivo se compone de bloques consecutivos con la siguiente estructura:

```
<PATH>
<FILE_NAME>
<FILE_NAME_2>
...
<PATH_2>
<FILE_NAME>
...

```

Donde:
<PATH>
    Es una línea de texto que termina en "/".
    Establece el directorio base de trabajo para las líneas
    subsiguientes. (ej. /etc/services/).

<FILE_NAME>
    Es una línea de texto que NO CONTIENE "/".
    Representa el nombre de un ejecutable o script ubicado en
    el <PATH> actual (ej. dbus, tty1).

#### Señales admitidas

El supervisor pude recibir las siguientes señales:

- SIGUSR1 (10): Le indica al supervisor que debe terminar la ejecución de su servicio. El supervisor envía `SIGTERM` en primera instancia y `SIGKILL` si el servicio se reusa a terminar su ejecución a tiempo.

El init puede recibir las siguientes señales:

- SIGUSR1 (10): Inicia la secuencia de apagado del sistema. Se puede usar el wrapper `power-off` como utilidad para este propósito.

### Usage Manual

The current system is configured to operate in two distinct modes: using the supervisor alongside the init, or operating solely with the init and leaving the system's supervision to the administrator's discretion. It is recommended to pair SimpleInit with Supervised to take advantage of this scheme's maximum functionality and it is the configuration expected by the manual.

#### Required Files

SimpleInit and Supervised each consist of a single binary; both reside in `/bin`. Additionally, the SimpleInit binary must be set in the system configuration as PID 1. The configuration files are stored in `/etc/sim-init/`, consisting of `r0`, `basics`, and `shutdown`. The `basics` and `shutdown` files must be executable; in the example, they are bash scripts. `basics` performs the initial configuration and all _"basic"_ tasks for the system's functionality, whereas `shutdown` prepares the system to power off before SimpleInit calls the kernel to power-off the machine. System runlevels are configured in `rN` files, where N is the level. Currently, there is only support for a single runlevel, so only `r0` is needed in the directory. Some system utilities will be stored in `/bin/` too. Therefore, the file system structure will look like this:

```
/bin/
|   sim-init
|   supervised
|   power-off
...
/etc/
|   sim-init/
|   |       r0
|   |       basics
|   |       shutdown

```

The r0 file is configured with the following format:

The file syntax consists of consecutive blocks with the following structure:

```
<PATH>
<FILE_NAME>
<FILE_NAME_2>
...
<PATH_2>
<FILE_NAME>
...
```

Where:
<PATH>
    Is a line of text that ends with "/".
    It establishes the base working directory for the subsequent
    lines (e.g., /etc/services/).

<FILE_NAME>
    Is a line of text that DOES NOT CONTAIN "/".
    It represents the name of an executable or script located in
    the current <PATH> (e.g., dbus, tty1).

#### Admitted signals

The supervisor can handle the following signals:

- SIGUSR1 (10): It directs the supervisor to terminate its own service. The supervisor first attempts to send `SIGTERM`, if the service refuses to stop after a set time it force-stops the process with `SIGKILL`.

The init can handle the following signals:

- SIGUSR1 (10): It initiates the system's shutdown sequence. The `power-off` wrapper utility can be used for this purpouse.


