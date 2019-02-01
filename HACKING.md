# Working on the DIGITAiL App

## Coding style

The coding style employed in the DIGITAiL codebase is
[the KDELibs codestyle](https://community.kde.org/Policies/Kdelibs_Coding_Style)

## Code Layout

Note that the following does not include code function documentation as such,
rather it focuses on overview. See the header files for each specific class
for the documentation for each specific class

### Basic filesystem layout

* **(root)** The root folder, containing the base build information, and the main CMakeLists.txt project file
** **3rdparty** Contains external code (in particular the Kirigami Qt Quick UI framework)
** **data** The Android specific parts of the codebase, and Android gradle build instructions
** **src** The sourcecode for both the service and the frontend

### Code Concepts

The code is split in two main parts:

* Frontend application (based on the Kirigami Qt Quick UI framework)
* Backend service (QCoreApplication on everything but Android, where it is a QAndroidService)

The interaction between the backend and frontend is done through objects replicated
through Qt's RemoteObjects module.

### Backend/Service

The service is launched with the command '''digitail -service'''

Upon launching the service, a number of objects are created and exposed through a local
RemoteObjects node. The exposed object classes, in hierarchical layout, are:

* **AppSettings** Contains basic settings for the application, such as advanced mode, and the various idle mode settings (and future settings such as tail name and the like)
* **BTConnectionManager** Central interface for connecting to tails, exposing both connection functions, and the three further convenience objects listed below:
** **BTDeviceModel** A QAbstractListModel containing bluetooth devices for display if more than one tail is found
** **TailCommandModel** Once BTConnectionManager has connected to a tail, this model is automatically filled with commands, according to the version of the tail
** **CommandQueue** The main interface for sending commands to the tail. This supports sending any number of commands in order, and further supports list manipulation options to adjust the queue.

It further includes the IdleMode class, which handles queueing up random
commands and pauses, if the mode is turned on in AppSettings. This class is not
exported, as it is controlled indirectly and operates without user interaction.

### Frontend

The frontend is launched with the command '''digitail'''

Upon launching the application, it will first create the QML application engine,
and register a few types on it (primarily the Kirigami types, and some
convenience types). It will then attempt to connect to the RemoteObjects node,
and acquire the replication objects. These objects are then registered as
context property objects on the engine's root context. Finally, the engine is
asked to load the application main qml file, which in turn will show the main
window. All communication between the frontend and backend is done through these
objects.

The base layout of the frontend's UI is:

* **main.qml** Contains the root Kirigami ApplicationWindow, which holds a number of pages, as well as the logic of the global context drawer (which allows for switching directly between WelcomePage, TailMoves, TailLights, IdleModePage and AboutPage).
** **qml/AboutPage.qml** A simple informational page which shows support, version and licensing information about the application, as well as contributor details.
** **qml/BaseMovesPage.qml** A page containing a list of cards
** **qml/ConnectToTail.qml** A sheet which can be shown to the user, listing all found tails. This should only be shown if more than one tail has been discovered.
** **qml/IdleModePage.qml** Settings for idle mode: The range of duration of the pause between random commands, and the categories to include in the selection.
** **qml/NotConnectedCard.qml** A component which gives the user sensible options to perform when not connected to a tail.
** **qml/TailLights.qml** A specific instance of BaseMovesPage which shows the single lighting category  of tail commands.
** **qml/TailMoves.qml** A specific instance of BaseMovesPage which shows the three movement categories of tail commands.
** **qml/WelcomePage.qml* The first page shown to the user upon launching the application. This shows a grid of functional options (alarm, tail poses editor, moves playlist, tail lights, and tail moves). If there is no connected tail, an instance of the NotConnectedCard is shown at the top. If the user is connected to a tail, a tickbox shown in a card underneath the function grid allows the user to turn on idle mode, and when turned on allows the user to switch to the idle mode settings page.
