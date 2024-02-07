This project can help researchers simulate drones under OMNeT++. Moreover, we describe the use of the INET framework to create a scenario containing communication between drones by sending and exchanging messages between them using the UDP application. However, it is much more interested in the integration of UAV-cloud into a FANET network that presents many technical and conceptual challenges.

1. **Resource Management Mechanisms:**
   - It is essential to develop effective resource management mechanisms to ensure optimal resource allocation to Client Drones (CDs). This involves taking into account the specific constraints linked to drones, such as their limited computing capacities and their energy constraints.

2. **Quality of Service (QoS) Variation:**
   - The Quality of Service (QoS) varies from one Provider of Drones (PDs) to another. Thus, CDs attempt to select the most appropriate services.

3. **Ground Station (GS) and Game Theory:**
   - This work is interested in using Ground Station (GS) that acts as the repository of offered and requested services. Game theory can be applied as the mathematical model for service selection inside the GS, especially in interactive decision-making, where each participant's outcome is influenced by the actions of others.

4. **Application of Game Theory:**
   - Game theory can assist drone clients in discovering and selecting the best services for their needs. CDs naturally expect to receive as many services as possible within their limited budgets, while PDs attempt to maximize their utility by incentivizing clients to pay for their limited number of services.

5. **Service Discovery and Selection:**
   - Before applying, the services must be discovered by clients, and clients must also be able to learn about the different services available. CDs choose the best service and suitable providers in terms of QoS and service price.

Here are two photos that illustrate where to change your OMNeT++:

 ![image01](https://github.com/moumouh42/UAV-Cloud-with-game-theory-inOMNeT-4.6/assets/153837902/1b002022-0225-4d08-a0bf-9f84d8ba254f)
 ![image02](https://github.com/moumouh42/UAV-Cloud-with-game-theory-inOMNeT-4.6/assets/153837902/e573f6b6-4fb1-441a-878b-3d55af079758)

And here is the video that shows a simulation with 15 supplier drones and 15 client drones, so a GS starts the game theory between them:

- [Simulation Video](https://github.com/moumouh42/UAV-Cloud-with-game-theory-inOMNeT-4.6/assets/153837902/d1fd049a-2de7-45d4-931d-a5e255f7c5a0)
