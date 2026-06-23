**Modelos Globales de Iluminación en la Computación Gráfica**

**Algoritmos, Fortalezas, Debilidades y Casos Prácticos de Aplicación**

[Apellidos y nombres del integrante 1]

[Apellidos y nombres del integrante 2]

[Apellidos y nombres del integrante 3]

[Apellidos y nombres del integrante 4]

Facultad de Ingeniería de Sistemas e Informática

Universidad Nacional Mayor de San Marcos

Computación Visual / Computación Gráfica

[Nombre del docente]

[Fecha de entrega]

**Resumen**

La síntesis de imágenes fotorrealistas es uno de los objetivos centrales de la computación gráfica, y depende en gran medida de la fidelidad con que se simula el comportamiento de la luz. Los modelos globales de iluminación agrupan un conjunto de algoritmos que reproducen no solo la luz que llega directamente desde las fuentes, sino también la que resulta de las múltiples reflexiones y refracciones entre las superficies de una escena, dando lugar a fenómenos como las sombras suaves, el sangrado de color, las cáusticas y la interreflexión difusa.

El presente informe describe los fundamentos de la iluminación global a partir de su distinción con los modelos locales y de la ecuación de renderizado formulada por Kajiya (1986), y analiza los principales métodos desarrollados a lo largo de su evolución: el trazado de rayos, la radiosidad, el trazado de trayectorias, el mapeo de fotones, las técnicas orientadas al tiempo real y, como cierre, el renderizado neuronal que ha emergido en los últimos años como una vía alternativa de síntesis de imágenes. Para cada método se exponen su funcionamiento, sus fortalezas, sus debilidades y sus casos prácticos de aplicación en áreas como la visualización arquitectónica, los videojuegos y el cine, y se discuten además las limitaciones que el campo aún no resuelve de forma satisfactoria.

Se concluye que no existe un algoritmo universalmente superior, sino que la elección depende del equilibrio buscado entre realismo físico, costo computacional y restricciones de tiempo, y que la tendencia actual combina la aceleración por hardware y el aprendizaje profundo para acercar la iluminación global físicamente correcta a las aplicaciones interactivas.

*Palabras clave:* iluminación global, trazado de rayos, radiosidad, trazado de trayectorias, mapeo de fotones, renderizado neuronal, renderizado fotorrealista.

**Índice**

*Nota para el equipo: en Word, hacer clic derecho sobre el índice y seleccionar “Actualizar campo” → “Actualizar todo el índice” para que se generen los números de página automáticamente.*

**Distribución de la Exposición**

El trabajo se repartió entre los cuatro integrantes del equipo procurando que cada uno desarrollara un bloque temático coherente y de extensión equivalente, de modo que pueda exponerlo con dominio suficiente. La Tabla 1 detalla las secciones y los temas asignados a cada integrante; los apartados de cada bloque se han redactado con la profundidad necesaria para sustentar la exposición oral correspondiente.

**Tabla 1**

*Distribución de los temas de exposición por integrante*

| **Integrante** | **Secciones** | **Temas que expondrá** |
| --- | --- | --- |
| Integrante 1 | 1 y 2 | Introducción; fundamentos de la iluminación global; diferencia entre iluminación local y global; ecuación de renderizado |
| Integrante 2 | 3.1, 3.2 y 5.1 | Trazado de rayos; radiosidad; aplicación en visualización arquitectónica |
| Integrante 3 | 3.3, 3.4 y 5.3 | Trazado de trayectorias; mapeo de fotones; aplicación en cine y efectos visuales |
| Integrante 4 | 3.5, 3.6, 4, 5.2, 6 y 7 | Iluminación global en tiempo real; renderizado neuronal; comparación de los modelos; aplicación en videojuegos; limitaciones; conclusiones |

*Nota.* Elaboración propia. El reparto se organizó por bloques temáticos afines para equilibrar la carga de exposición entre los integrantes.

# **1. Introducción**

Desde sus inicios, la computación gráfica ha perseguido la generación de imágenes capaces de aproximarse a una fotografía o, idealmente, de resultar indistinguibles de la reality. Alcanzar ese grado de realismo exige reproducir con fidelidad la manera en que la luz interactúa con las superficies de una escena: cómo se refleja, se refracta, se absorbe y se propaga de un objeto a otro. El conjunto de cálculos que describe esta interacción se conoce como modelo de iluminación, y de su sofisticación depende, en buena medida, la calidad visual del resultado.

Los primeros modelos, denominados de iluminación local, consideran únicamente la luz que llega directamente desde las fuentes hacia cada superficie, tratando a los objetos como entidades aisladas. Aunque son rápidos y suficientes para numerosas aplicaciones, no representan adecuadamente fenómenos cotidianos como la iluminación indirecta o el color que una superficie transfiere a otra cercana. Para superar estas limitaciones surgieron los modelos globales de iluminación, que toman en cuenta la luz que ha sufrido una o más reflexiones antes de llegar al observador.

La historia de estos métodos se remonta a la década de 1980, con la introducción del trazado de rayos por Whitted (1980) y de la radiosidad por Goral et al. (1984). Un fundamento teórico unificador llegó en 1986, cuando Kajiya formuló la ecuación de renderizado, que generaliza una amplia variedad de algoritmos previos y orienta la investigación posterior (Kajiya, 1986). Desde entonces, la iluminación global ha evolucionado de ser un proceso reservado a costosas granjas de cálculo a estar disponible, en formas simplificadas, en aplicaciones interactivas (Aguirre Pérez, 2024), y más recientemente ha comenzado a dialogar con representaciones de escena aprendidas a partir de fotografías (Mildenhall et al., 2020; Kerbl et al., 2023).

Este informe tiene como objetivo describir los principales modelos globales de iluminación, analizando para cada uno su funcionamiento, sus fortalezas, sus debilidades y sus casos prácticos de aplicación. El documento se organiza en un marco conceptual que distingue la iluminación local de la global y presenta la ecuación de renderizado; una descripción de los algoritmos más representativos, incluyendo las tendencias recientes de renderizado neuronal; una comparación sintética entre ellos; una revisión de sus aplicaciones; una discusión de las limitaciones que el campo aún no resuelve; y, finalmente, las conclusiones.

# **2. Marco Conceptual de la Iluminación**

## **2.1. Iluminación Local frente a Iluminación Global**

La iluminación local calcula el color de un punto de una superficie considerando solo la luz que proviene directamente de las fuentes, junto con las propiedades del material y, normalmente, una componente de luz ambiente constante que aproxima de forma muy simplificada la luz indirecta. Modelos clásicos como el de Phong combinan términos difusos y especulares para producir resultados convincentes a bajo costo, razón por la cual constituyen la base del sombreado en tiempo real. Sin embargo, al tratar cada objeto de forma independiente, ignoran la interacción de la luz entre las superficies, lo que produce sombras planas y una pérdida de realismo en los espacios cerrados (Departamento de Informática, s.f.).

La iluminación global, en cambio, modela el transporte de la luz a través de toda la escena: además de la iluminación directa, incorpora la luz indirecta que rebota de una superficie a otra. Gracias a ello reproduce efectos que la iluminación local no puede capturar, como las sombras suaves, el sangrado de color -cuando una superficie tiñe a otra próxima con su color-, las cáusticas producidas por la concentración de luz a través de objetos transparentes, y la interreflexión difusa que ilumina las zonas no expuestas directamente a las fuentes (Aguirre Pérez, 2024). El costo de esta fidelidad es un esfuerzo computacional considerablemente mayor.

Conviene precisar que la luz que recibe un punto puede descomponerse en una componente directa, procedente sin obstáculos de las fuentes, y una componente indirecta, formada por la luz que ha rebotado al menos una vez en otras superficies antes de alcanzarlo. Los modelos locales aproximan únicamente la primera y sustituyen la segunda por un término ambiente constante; los modelos globales calculan explícitamente ambas. Por ello la iluminación global suele expresarse como la suma de la iluminación directa y de la contribución de todos los rebotes indirectos sucesivos, cuyo aporte decrece con cada rebote pero no desaparece, y es precisamente esa contribución acumulada la que confiere realismo a la escena.

**Figura 1**

*Comparación entre iluminación local y global*

|  |
| --- |
| **[ Pega aquí la imagen: diagrama o par de renders que contraste una escena con iluminación local (sombras planas) y con iluminación global (luz indirecta y sangrado de color) ]** |

Imagen sugerida en: <https://es.wikipedia.org/wiki/Iluminaci%C3%B3n_global>

*Nota.* Adaptado de “Iluminación global”, en Wikipedia, la enciclopedia libre, bajo licencia CC BY-SA. Puede reemplazarse por un render propio generado en Blender u otra herramienta.

## **2.2. La Ecuación de Renderizado**

El marco teórico que sustenta la iluminación global es la ecuación de renderizado propuesta por Kajiya (1986). Esta ecuación integral describe la radiancia que abandona un punto de una superficie en una dirección dada como la suma de dos términos: la luz que el propio punto emite y la integral, sobre todas las direcciones del hemisferio, de la luz que incide sobre él, ponderada por la función de reflectancia bidireccional del material y por un factor coseno que considera la orientación de la superficie. De manera simplificada puede escribirse:

*Lₒ(x, ωₒ) = Lₑ(x, ωₒ) + ∫Ω fᵣ(x, ωᵢ, ωₒ) · Lᵢ(x, ωᵢ) · (ωᵢ · n) dωᵢ*

La importancia de esta formulación radica en que la mayoría de los algoritmos de iluminación global pueden entenderse como aproximaciones numéricas a su solución. Como la integral no admite una solución analítica en escenas reales, cada método introduce simplificaciones o estrategias de muestreo distintas: la radiosidad la resuelve para superficies difusas mediante elementos finitos, mientras que el trazado de trayectorias y el mapeo de fotones emplean métodos estocásticos de Monte Carlo (Aguirre Pérez, 2024; Kajiya, 1986).

Un aspecto esencial es que la radiancia incidente que aparece dentro de la integral es, a su vez, la radiancia saliente de otros puntos de la escena. La ecuación queda así definida de forma recursiva: para conocer la luz en un punto haría falta conocerla en todos los demás. Esta interdependencia, sumada a que la integral carece de solución analítica en escenas reales, explica por qué la iluminación global exige métodos numéricos y resulta intrínsecamente costosa, y por qué los distintos algoritmos se diferencian sobre todo en la estrategia con que aproximan dicha recursión.

La función fᵣ -la BRDF, o función de distribución de reflectancia bidireccional- puede modelarse con distintos niveles de sofisticación. Los modelos clásicos como el de Phong son aproximaciones empíricas, sin base física rigurosa, que bastan para superficies plásticas simples pero no capturan con precisión cómo los metales, las superficies pulidas o los materiales rugosos reflejan la luz. Desde la década de 1980 se han desarrollado modelos de reflectancia basados en la física de las microfacetas, que representan cada superficie como un conjunto de espejos microscópicos orientados aleatoriamente. El modelo de Cook y Torrance (1982) fue pionero en introducir este enfoque en la computación gráfica, y trabajos posteriores como el de Walter et al. (2007), que popularizó la distribución GGX, lo refinaron hasta convertirlo en el estándar de los flujos de trabajo de renderizado físicamente basado (PBR) empleados hoy tanto en producción cinematográfica como en motores de videojuegos. Estos modelos describen la reflectancia mediante tres componentes -una función de distribución de normales, un término de sombreado-oclusión entre microfacetas y un término de Fresnel que pondera la reflexión según el ángulo de incidencia-, lo que permite representar de forma unificada desde superficies mate hasta metales pulidos.

# **3. Modelos y Algoritmos de Iluminación Global**

## **3.1. Trazado de Rayos (Ray Tracing)**

El trazado de rayos parte de la idea de seguir el recorrido de la luz en sentido inverso al físico: en lugar de simular los rayos que salen de las fuentes, traza rayos desde la cámara, uno o varios por cada píxel de la imagen, hacia el interior de la escena. Para cada rayo se calcula la intersección más cercana con la geometría y, en ese punto, Whitted (1980) propuso lanzar de forma recursiva nuevos rayos: rayos de reflexión, que simulan los espejos y superficies brillantes; rayos de refracción o transmisión, que atraviesan los materiales transparentes; y rayos de sombra, dirigidos hacia las fuentes de luz para determinar si el punto se encuentra iluminado u ocluido.

Este esquema recursivo permite reproducir con precisión reflexiones y refracciones especulares, así como sombras nítidas. Posteriormente, Cook et al. (1984) generalizaron la técnica mediante el trazado de rayos distribuido, que utiliza muestreo estocástico para incorporar efectos adicionales como las sombras suaves, el desenfoque de movimiento, la profundidad de campo y los reflejos borrosos, ampliando el repertorio de fenómenos representables.

Dado que una sola imagen puede requerir millones de rayos y una escena, millones de polígonos, comprobar cada rayo contra toda la geometría sería inviable. Por ello el trazado de rayos se apoya en estructuras de aceleración espacial -como las jerarquías de volúmenes envolventes (BVH) o los árboles kd-, que descartan con rapidez la geometría irrelevante y reducen drásticamente el número de pruebas de intersección. A diferencia de la rasterización, que proyecta los polígonos sobre la pantalla y resuelve la visibilidad píxel a píxel sin información del resto de la escena, el trazado de rayos consulta la geometría global en cada punto; en ello reside tanto su mayor realismo como su mayor costo.

**Fortalezas.** Genera reflexiones y refracciones físicamente convincentes, sombras precisas y un realismo difícil de alcanzar con la rasterización tradicional. Su formulación es conceptualmente sencilla, se adapta de forma natural al procesamiento en paralelo y constituye la base sobre la que se construyen métodos más avanzados como el trazado de trayectorias y el mapeo de fotones.

**Debilidades.** Su costo computacional es elevado, pues el número de rayos crece rápidamente con cada nivel de recursión. En su forma clásica modela bien los efectos especulares, pero no captura adecuadamente la interreflexión difusa -la luz indirecta entre superficies mate- ni representa de manera directa las fuentes de luz de área, que suelen aproximarse mediante conjuntos de fuentes puntuales (Departamento de Informática, s.f.).

**Casos prácticos de aplicación.** Se ha utilizado durante décadas en la generación de imágenes por computadora para cine y televisión y en la visualización de productos. En la actualidad, gracias a la aceleración por hardware, se emplea también en videojuegos para mejorar reflejos y sombras en tiempo real (Machado de Benedetti, 2021).

**Figura 2**

*Esquema del trazado de rayos recursivo*

|  |
| --- |
| **[ Pega aquí la imagen: diagrama que muestre el rayo primario desde la cámara, los rayos de reflexión y refracción, y los rayos de sombra hacia la fuente de luz ]** |

Imagen sugerida en: <https://es.wikipedia.org/wiki/Trazado_de_rayos>

*Nota.* Adaptado de “Trazado de rayos”, en Wikipedia, la enciclopedia libre, bajo licencia CC BY-SA.

## **3.2. Radiosidad (Radiosity)**

La radiosidad aborda el problema desde una perspectiva opuesta al trazado de rayos. En lugar de seguir rayos individuales, modela el intercambio de energía radiante entre las superficies de la escena, Muscle partiendo de los principios de la transferencia de calor por radiación. Goral et al. (1984) aplicaron este enfoque dividiendo la escena en pequeños parches y suponiendo que todas las superficies son difusas o lambertianas; es decir, que reflejan la luz por igual en todas las direcciones.

El método calcula, para cada par de parches, un factor de forma que cuantifica qué proporción de la energía que abandona uno alcanza al otro, y plantea un sistema de ecuaciones lineales cuya solución proporciona la radiosidad de cada parche. Una característica distintiva es que el resultado es independiente del punto de vista: dado que solo depende de la geometría y de la iluminación, una vez calculado puede observarse desde cualquier ángulo sin necesidad de repetir el cómputo.

Formalmente, la radiosidad de cada parche se expresa como la suma de su emisión propia y de la fracción reflejada -según su reflectividad- de la radiosidad que recibe del resto de parches, ponderada por los factores de forma:

*Bᵢ = Eᵢ + ρᵢ · Σⱼ Bⱼ · Fᵢⱼ*

donde B es la radiosidad, E la emisión, ρ la reflectividad y F el factor de forma entre dos parches. Resolver directamente el sistema de ecuaciones resultante es costoso, por lo que en la práctica se emplean métodos iterativos: el refinamiento progresivo distribuye en cada paso la energía del parche más brillante, lo que permite mostrar una solución aproximada de forma temprana, mientras que la radiosidad jerárquica agrupa los parches lejanos para reducir el número de interacciones que deben calcularse.

**Fortalezas.** Reproduce con gran realismo la interreflexión difusa, el sangrado de color y las sombras suaves, efectos especialmente notorios en interiores. Al ser independiente de la vista, su resultado puede precalcularse y almacenarse en mapas de iluminación reutilizables en escenas estáticas, lo que ahorra cómputo durante la visualización. Además, maneja de forma natural las fuentes de luz de área.

**Debilidades.** Solo modela la reflexión difusa, por lo que no representa reflejos especulares ni refracciones y debe combinarse con otras técnicas para lograrlos. El cálculo de los factores de forma es costoso -del orden del cuadrado del número de parches- y exige un consumo de memoria elevado. Asimismo, resulta poco práctica en escenas con geometría que cambia de forma dinámica.

**Casos prácticos de aplicación.** Se emplea en visualización arquitectónica y en estudios de iluminación, así como en la generación de iluminación precalculada para motores de videojuegos, donde la luz indirecta se hornea en mapas de luz aplicados luego en tiempo real.

**Figura 3**

*Radiosidad y sangrado de color en la caja de Cornell*

|  |
| --- |
| **[ Pega aquí la imagen: render de la caja de Cornell que evidencie el sangrado de color de las paredes sobre los objetos y las sombras suaves ]** |

Imagen sugerida en: [https://es.wikipedia.org/wiki/Radiosidad\_(inform%C3%A1tica)](https://es.wikipedia.org/wiki/Radiosidad_%28inform%C3%A1tica%29)

*Nota.* Adaptado de “Radiosidad (informática)”, en Wikipedia, la enciclopedia libre, bajo licencia CC BY-SA. La caja de Cornell es una escena de prueba estándar en la investigación de iluminación global.

## **3.3. Trazado de Trayectorias (Path Tracing)**

El trazado de trayectorias es una extensión del trazado de rayos concebida por Kajiya (1986) como método para resolver la ecuación de renderizado mediante integración de Monte Carlo. En lugar de limitarse a los rayos de reflexión y refracción especular, en cada punto de intersección muestrea aleatoriamente una dirección de entre todas las posibles, construyendo trayectorias completas que conectan la cámara con las fuentes de luz a través de varios rebotes, incluidos los difusos.

Al promediar un gran número de estas trayectorias aleatorias por píxel, el método converge hacia la solución correcta de la ecuación de renderizado. Por construcción es no sesgado, lo que significa que, con suficientes muestras, su resultado tiende al valor físicamente exacto. La principal dificultad práctica es que la varianza del muestreo se manifiesta como ruido -un granulado característico- que disminuye con lentitud a medida que se incrementa el número de muestras (Aguirre Pérez, 2024).

Para que el método resulte eficiente se incorporan varias técnicas de reducción de varianza. El muestreo por importancia concentra las direcciones muestreadas allí donde la función de reflectancia aporta más energía; la estimación del siguiente evento evalúa en cada rebote la contribución directa de las fuentes, sin esperar a que una trayectoria las alcance por azar; y la ruleta rusa termina de forma probabilística las trayectorias poco influyentes, acotando su longitud sin introducir sesgo. Aun así, la convergencia es lenta: el error disminuye con la raíz cuadrada del número de muestras, de modo que reducir el ruido a la mitad exige cuadruplicar el cómputo.

Una técnica adicional, anterior al propio trazado de trayectorias y compatible con él, es el caché de irradiancia propuesto por Ward et al. (1988): dado que la componente difusa de la iluminación indirecta varía suavemente sobre la mayoría de las superficies, basta con calcularla en un conjunto disperso de puntos e interpolar el resultado en el resto de la imagen, lo que reduce notablemente el número de muestras costosas necesarias para estimar la interreflexión difusa.

**Fortalezas.** Captura en un único marco unificado prácticamente todos los efectos del transporte de la luz: iluminación indirecta difusa, sombras suaves, profundidad de campo y reflejos. Es físicamente correcto, relativamente sencillo de implementar y escala bien al aumentar la capacidad de cómputo, lo que lo ha convertido en el estándar de la producción cinematográfica.

**Debilidades.** Requiere un número muy elevado de muestras para eliminar el ruido, lo que se traduce en tiempos de cálculo prolongados. Determinados caminos de luz, como las cáusticas o las combinaciones de reflexión especular y difusa, resultan especialmente difíciles de muestrear de forma eficiente. Variantes como el trazado de trayectorias bidireccional y el transporte de luz de Metropolis buscan mitigar estas limitaciones.

**Casos prácticos de aplicación.** Es la base de los motores de renderizado para cine y animación, como Arnold, RenderMan y Cycles de Blender. Más recientemente, apoyado en la reducción de ruido y en el reescalado por aprendizaje profundo, ha llegado a los videojuegos en títulos como Cyberpunk 2077 y Alan Wake 2 (Gutiérrez Rodríguez, 2022).

**Figura 4**

*Convergencia del trazado de trayectorias según el número de muestras*

|  |
| --- |
| **[ Pega aquí la imagen: secuencia de la misma escena renderizada con un número creciente de muestras por píxel, mostrando cómo el ruido disminuye progresivamente ]** |

Imagen sugerida en: <https://en.wikipedia.org/wiki/Path_tracing>

*Nota.* Adaptado de “Path tracing”, en Wikipedia, the free encyclopedia, bajo licencia CC BY-SA.

## **3.4. Mapeo de Fotones (Photon Mapping)**

El mapeo de fotones, desarrollado por Jensen (1996), es un algoritmo de iluminación global que opera en dos pasadas. En la primera, se emiten fotones desde las fuentes de luz y se traza su recorrido por la escena; cada vez que un fotón impacta una superficie, su energía y posición se almacenan en una estructura denominada mapa de fotones. En la segunda pasada, se realiza un renderizado mediante trazado de rayos en el que la radiancia de cada punto se estima a partir de la densidad de fotones almacenados en su vecindad.

Esta separación entre el transporte de la luz y la geometría de la cámara permite reutilizar el mapa de fotones desde distintos puntos de vista y resolver con eficiencia efectos que resultan costosos para el trazado de trayectorias puro. El mapeo de fotones es un método sesgado, ya que la estimación por densidad introduce un ligero desenfoque, pero ofrece resultados con menos ruido en los fenómenos para los que está especialmente diseñado (Aguirre Pérez, 2024).

En la práctica suelen construirse dos mapas distintos: un mapa de cáusticas, con una alta densidad de fotones, dedicado a la luz concentrada a través de superficies especulares o refractantes, y un mapa global, más disperso, para el resto de la iluminación indirecta. La búsqueda de los fotones más cercanos a cada punto se acelera mediante un árbol kd. Para evitar el desenfoque en las zonas difusas, la estimación por densidad suele combinarse con una etapa de recolección final, que muestrea el entorno de cada punto a partir del mapa global y mejora la calidad del resultado a cambio de un mayor costo de cálculo.

**Fortalezas.** Resuelve de manera eficiente las cáusticas, la dispersión subsuperficial propia de materiales translúcidos como la piel o la cera, y la iluminación en medios participativos como el humo o la niebla. Al desacoplar la iluminación de la cámara, se adapta bien a escenas en las que estos efectos serían difíciles de capturar con muestreo directo.

**Debilidades.** Introduce sesgo y, por tanto, un cierto desenfoque que debe controlarse cuidadosamente. Su calidad depende del número de fotones y del radio de búsqueda elegidos, parámetros cuyo ajuste inadecuado genera artefactos visibles. Además, el almacenamiento del mapa de fotones puede demandar una cantidad considerable de memoria. Para mitigar estas limitaciones, Hachisuka et al. (2008) propusieron el mapeo de fotones progresivo, que en lugar de almacenar un único mapa de gran tamaño antes de renderizar, acumula sucesivas rondas de fotones y reduce de manera progresiva el radio de búsqueda en cada paso, lo que permite alcanzar una solución consistente -que converge al resultado correcto a medida que aumenta el número de fotones- sin las limitaciones de memoria del método clásico, a costa de un mayor número de pasadas de renderizado.

**Casos prácticos de aplicación.** Se utiliza en la producción cinematográfica para representar cáusticas y materiales translúcidos, con frecuencia en combinación con el trazado de trayectorias dentro de soluciones híbridas que aprovechan las ventajas de ambos enfoques.

**Figura 5**

*Cáusticas generadas mediante mapeo de fotones*

|  |
| --- |
| **[ Pega aquí la imagen: render de un objeto transparente (por ejemplo, una copa de vidrio o agua) que muestre las cáusticas proyectadas sobre una superficie ]** |

Imagen sugerida en: <https://en.wikipedia.org/wiki/Photon_mapping>

*Nota.* Adaptado de “Photon mapping”, en Wikipedia, the free encyclopedia, bajo licencia CC BY-SA.

## **3.5. Iluminación Global en Tiempo Real**

Llevar la iluminación global a las aplicaciones interactivas plantea un reto adicional: la solución debe obtenerse dentro del escaso presupuesto de unos pocos milisegundos por fotograma. Para ello se han desarrollado diversas estrategias que sacrifican parte de la exactitud a cambio de velocidad. La iluminación precalculada u horneada almacena la luz indirecta en mapas de iluminación y sondas, ofreciendo gran calidad a costo casi nulo durante la ejecución, aunque con dificultades para los objetos en movimiento.

Entre las primeras estrategias pensadas específicamente para acercar la iluminación global al tiempo real se encuentra la radiosidad instantánea (instant radiosity), propuesta por Keller (1997). El método dispara un número reducido de fotones desde las fuentes de luz y, en cada punto de impacto, coloca una fuente puntual virtual (virtual point light, VPL) que reemite la energía recibida; la escena se renderiza después sumando la contribución de todas las fuentes virtuales mediante rasterización estándar. Al evitar el muestreo de trayectorias completas, la radiosidad instantánea ofrece una aproximación razonable de la interreflexión difusa a un costo mucho menor que el trazado de trayectorias, y sentó las bases de numerosas técnicas posteriores de iluminación indirecta en videojuegos.

Otra estrategia influyente es la transferencia de radiancia precalculada (precomputed radiance transfer, PRT), introducida por Sloan et al. (2002). El método precalcula, para cada punto de una geometría estática, cómo se transfiere la luz incidente -incluyendo sombras suaves e interreflexión- hacia la radiancia saliente, y codifica esa función de transferencia en una base compacta, típicamente armónicos esféricos. En tiempo de ejecución basta con proyectar la iluminación del entorno sobre esa misma base y combinarla con los coeficientes precalculados, lo que permite iluminación indirecta de alta calidad bajo entornos de luz que cambian dinámicamente, siempre que la geometría no se modifique. Por esta razón, la PRT ha sido históricamente popular en videojuegos con escenarios mayormente estáticos.

Dentro de las técnicas en espacio de pantalla, la oclusión ambiental merece una mención aparte por su uso extendido. Propuesta originalmente por Zhukov et al. (1998) y popularizada en la producción cinematográfica por Landis (2002), la oclusión ambiental no calcula transporte de luz indirecta real, sino que estima, para cada punto, qué fracción del hemisferio que lo rodea está bloqueada por geometría cercana, oscureciendo así las zonas más encerradas -esquinas, grietas, intersticios entre objetos- de un modo que se asemeja visualmente al efecto de la luz indirecta sin requerir su cálculo explícito. Su variante en espacio de pantalla (screen-space ambient occlusion, SSAO) estima esta oclusión a partir del búfer de profundidad ya disponible en el renderizado, lo que la vuelve prácticamente gratuita en términos de cómputo y, por ello, casi omnipresente en los motores de videojuegos actuales. Otras técnicas en espacio de pantalla, como la iluminación global en espacio de pantalla, extienden esta misma idea a la luz indirecta de color, aunque ambas quedan limitadas a la información visible en la imagen.

Otras aproximaciones reconstruyen la geometría de la escena con vóxeles para propagar la luz de forma dinámica, como en el trazado de conos por vóxeles (Aguirre Pérez, 2024). El avance más determinante de los últimos años ha sido la incorporación de hardware especializado en el trazado de rayos -como los núcleos RTX de NVIDIA y la interfaz DirectX Raytracing-, que acelera el cálculo de intersecciones y hace viable un trazado de rayos parcial en tiempo real (Machado de Benedetti, 2021). Este se combina con técnicas de reducción de ruido, de muestreo eficiente y de reescalado por aprendizaje profundo que permiten reconstruir imágenes de calidad a partir de un número reducido de muestras (Gutiérrez Rodríguez, 2022). La tendencia dominante es, en consecuencia, el renderizado híbrido que une la rasterización con el trazado de rayos para equilibrar calidad y rendimiento.

Entre estas estrategias destacan las sondas de irradiancia distribuidas por la escena, que almacenan la luz indirecta en puntos discretos y la interpolan para iluminar los objetos en movimiento, y los métodos recientes de remuestreo por importancia espacio-temporal, que reutilizan la información de los píxeles vecinos y de los fotogramas anteriores para estimar la contribución de numerosas fuentes de luz con muy pocas muestras por píxel. Estas técnicas, combinadas con la reducción de ruido y el reescalado por aprendizaje profundo, han hecho posible una iluminación global dinámica de calidad creciente dentro del presupuesto de tiempo de un videojuego.

## **3.6. Renderizado Neuronal y Representaciones Implícitas de la Escena**

La incorporación del aprendizaje profundo a la iluminación global no se limita a la reducción de ruido o al reescalado de imágenes mencionados en la sección anterior: en los últimos años ha surgido una familia de métodos que reemplaza total o parcialmente la representación geométrica explícita de la escena -mallas de polígonos, vóxeles, parches- por una representación aprendida a partir de fotografías.

El caso más influyente es el de los campos de radiancia neuronales (neural radiance fields, NeRF), introducidos por Mildenhall et al. (2020). Un NeRF entrena una red neuronal pequeña para que, dado un punto del espacio y una dirección de observación, prediga la densidad y el color emitido en ese punto; integrando estas predicciones a lo largo de los rayos de cámara, de forma similar al trazado de rayos pero sobre un volumen continuo, es posible sintetizar vistas nuevas de una escena a partir de un conjunto disperso de fotografías. El resultado es un método capaz de reconstruir efectos de iluminación complejos -reflejos, transparencias, profundidad de campo- directamente de los datos, sin necesidad de especificar manualmente los materiales ni las fuentes de luz, aunque a costa de tiempos de entrenamiento y de renderizado considerables y de una capacidad limitada para el relighting, es decir, para modificar la iluminación de la escena ya reconstruida.

Para resolver el problema de la velocidad, Kerbl et al. (2023) propusieron el splatting de gaussianas en 3D (3D Gaussian splatting), que sustituye la red neuronal continua de los NeRF por un conjunto explícito de miles de elipsoides gaussianos con color y opacidad, optimizados directamente a partir de las fotografías de entrada. Al proyectar y combinar estos elementos mediante un proceso de rasterización diferenciable, el método alcanza calidades de síntesis de vistas comparables a las de los NeRF, pero con tiempos de entrenamiento de minutos y tasas de renderizado en tiempo real, lo que ha despertado un fuerte interés tanto en investigación como en aplicaciones de producción audiovisual, captura de escenarios reales y realidad virtual.

Estos métodos no resuelven la ecuación de renderizado de manera explícita como lo hacen los algoritmos descritos en las secciones anteriores; en su lugar, aprenden una función que reproduce sus efectos visuales a partir de observaciones. Por ello se les considera, más que un competidor directo del trazado de trayectorias o la radiosidad, un complemento: se emplean ya para acelerar el denoising de imágenes con ruido de Monte Carlo, para reconstruir la iluminación de un set real e insertar elementos generados por computadora de forma coherente -un uso habitual en la producción cinematográfica y los entornos de captura volumétrica-, y como punto de partida de investigación hacia una iluminación global que combine la corrección física de los métodos clásicos con la velocidad y la fidelidad fotográfica del aprendizaje profundo.

**Fortalezas.** Reconstruye efectos de iluminación complejos directamente a partir de fotografías, sin modelar manualmente materiales ni fuentes de luz; en el caso del splatting de gaussianas, alcanza además velocidades de renderizado en tiempo real.

**Debilidades.** Tiempos de entrenamiento considerables (especialmente en los NeRF), capacidad limitada para modificar la iluminación de la escena reconstruida, y necesidad de un conjunto numeroso de fotografías capturadas con buena cobertura de la escena.

**Casos prácticos de aplicación.** Captura y reconstrucción de escenarios reales para cine y realidad virtual, inserción coherente de elementos generados por computadora sobre material filmado, y aceleración de la reducción de ruido en renderizadores de trazado de trayectorias.

**Figura 6**

*Síntesis de nuevas vistas mediante campos de radiancia neuronales*

|  |
| --- |
| **[ Pega aquí la imagen: comparación entre fotografías de entrada y una vista nueva sintetizada mediante NeRF o 3D Gaussian splatting de la misma escena ]** |

Imagen sugerida en: <https://en.wikipedia.org/wiki/Neural_radiance_field>

*Nota.* Adaptado de “Neural radiance field”, en Wikipedia, the free encyclopedia, bajo licencia CC BY-SA.

# **4. Comparación de los Modelos**

La Tabla 2 resume las características principales de los modelos descritos, contrastando su fundamento, sus fortalezas, sus debilidades y sus aplicaciones típicas. La comparación evidencia que cada método responde mejor a determinadas necesidades, sin que ninguno resulte óptimo para todos los escenarios.

**Tabla 2**

*Comparación de los principales modelos globales de iluminación*

| **Modelo** | **Fundamento** | **Fortalezas** | **Debilidades** | **Aplicaciones** |
| --- | --- | --- | --- | --- |
| Trazado de rayos | Trazado recursivo de rayos desde la cámara (Whitted, 1980) | Reflejos y refracciones precisos; sombras nítidas | Costoso; no capta bien la luz indirecta difusa | Cine; visualización; juegos con RTX |
| Radiosidad | Intercambio de energía entre superficies difusas (Goral et al., 1984) | Interreflexión difusa y sangrado de color; independiente de la vista | Solo difusa; alto costo y memoria; poco dinámica | Arquitectura; mapas de luz precalculados |
| Trazado de trayectorias | Solución de Monte Carlo de la ecuación de renderizado (Kajiya, 1986) | Efectos de luz unificados; físicamente correcto | Ruido; requiere muchas muestras y tiempo | Cine y animación; juegos recientes |
| Mapeo de fotones | Dos pasadas: mapa de fotones y estimación de densidad (Jensen, 1996) | Cáusticas, translucidez y medios participativos | Sesgado; ajuste de parámetros; uso de memoria | Cáusticas y materiales translúcidos en cine |
| Renderizado neuronal | Representación aprendida de la escena a partir de fotografías (Mildenhall et al., 2020; Kerbl et al., 2023) | Fidelidad fotográfica; síntesis de vistas en tiempo real (Gaussian splatting) | Relighting limitado; requiere muchas fotografías de entrada | Captura de escenarios reales; cine y realidad virtual |

*Nota.* Elaboración propia a partir de Whitted (1980), Goral et al. (1984), Kajiya (1986), Jensen (1996), Mildenhall et al. (2020), Kerbl et al. (2023) y Aguirre Pérez (2024).

Más allá de los algoritmos en sí, conviene relacionarlos con el software en el que efectivamente se emplean, ya que la frontera entre renderizado de producción (offline) y renderizado en tiempo real determina en gran medida qué combinación de métodos resulta práctica. La Tabla 3 ofrece una selección no exhaustiva de motores y aplicaciones representativos de cada categoría.

**Tabla 3**

*Software y motores representativos de cada enfoque*

| **Categoría** | **Software / motor** | **Algoritmo principal** | **Ámbito típico** |
| --- | --- | --- | --- |
| Renderizado de producción (offline) | Blender (Cycles) | Trazado de trayectorias | Cine, animación, visualización |
| Renderizado de producción (offline) | Autodesk Arnold | Trazado de trayectorias | Cine, efectos visuales |
| Renderizado de producción (offline) | Chaos V-Ray | Trazado de trayectorias y radiosidad | Arquitectura, visualización de producto |
| Renderizado de producción (offline) | OTOY Octane | Trazado de trayectorias acelerado por GPU | Publicidad, motion graphics |
| Renderizado en tiempo real | Unreal Engine 5 (Lumen) | Trazado de conos por vóxeles y trazado de rayos híbrido | Videojuegos, producción virtual |
| Renderizado en tiempo real | Unity (HDRP) | Sondas de irradiancia, con trazado de rayos opcional (DXR) | Videojuegos |
| Renderizado en tiempo real | Blender (Eevee Next) | Aproximaciones en tiempo real con trazado de rayos por software | Previsualización, animación |

# **5. Casos Prácticos de Aplicación**

## **5.1. Visualización Arquitectónica y Diseño**

La iluminación global es especialmente valiosa en la visualización arquitectónica, donde la representación fiel de la luz natural y artificial resulta determinante. Herramientas profesionales basadas en trazado de rayos y radiosidad permiten a arquitectos e ingenieros simular la entrada de luz solar, evaluar la iluminación natural de los interiores y estimar la eficiencia energética de un diseño antes de su construcción, anclando la calidad de la imagen en la interacción realista de la luz con los materiales.

Más allá del valor estético, estas herramientas se utilizan como instrumento de análisis: permiten generar estudios de luz natural a lo largo del día y del año, mapas de luminancia en falso color y estimaciones de deslumbramiento que orientan tanto el diseño de los espacios como la certificación energética de los edificios. Dado que la radiosidad ofrece una solución independiente del punto de vista, resulta idónea para recorridos virtuales en los que el observador se desplaza por un interior cuya iluminación se calculó una sola vez.

## **5.2. Videojuegos y Gráficos en Tiempo Real**

En los videojuegos, la iluminación global ha pasado de aproximarse mediante mapas de luz precalculados a calcularse de forma dinámica gracias a la aceleración por hardware. Motores modernos combinan la rasterización con el trazado de rayos para incorporar reflejos, sombras y luz indirecta más realistas, apoyándose en técnicas de reducción de ruido y de reescalado por aprendizaje profundo para mantener tasas de fotogramas interactivas (Gutiérrez Rodríguez, 2022; Machado de Benedetti, 2021).

Los motores actuales incorporan sistemas de iluminación global dinámica que combinan un trazado por software sobre representaciones simplificadas de la escena con el trazado por hardware cuando está disponible, evitando hornear la luz y permitiendo que esta responda en tiempo de ejecución a los cambios de geometría, de hora del día o de las fuentes. Un ejemplo representativo es el sistema Lumen de Unreal Engine 5, que calcula reflejos y luz indirecta dinámicos sin iluminación precalculada. Este enfoque traslada al jugador efectos que antes solo eran posibles en el renderizado diferido, a costa de un cuidadoso equilibrio entre calidad y tasa de fotogramas.

## **5.3. Cine y Efectos Visuales**

La industria del cine y los efectos visuales constituye el ámbito donde la iluminación global se aplica con mayor exigencia de realismo. El trazado de trayectorias es hoy el estándar de los principales motores de producción, y se complementa con el mapeo de fotones cuando es necesario representar cáusticas o materiales translúcidos. Al no estar limitada por restricciones de tiempo real, la producción cinematográfica puede invertir el cómputo necesario para alcanzar imágenes físicamente correctas.

El paso de los esquemas de rasterización y de los métodos híbridos al trazado de trayectorias puro se consolidó a lo largo de la década de 2010, cuando los principales estudios adoptaron motores físicamente correctos para unificar en un mismo marco la iluminación de personajes, escenarios y efectos. Esta convergencia simplificó los flujos de trabajo -al eliminar la necesidad de ajustar manualmente la luz indirecta- y elevó el realismo, a cambio de tiempos de cálculo que se reparten en granjas de renderizado con cientos o miles de procesadores. Más recientemente, técnicas de captura basadas en NeRF y en splatting de gaussianas han comenzado a emplearse para reconstruir locaciones y actores reales con fines de producción virtual (Kerbl et al., 2023).

# **6. Limitaciones y Desafíos Abiertos**

A pesar de los avances descritos, la iluminación global físicamente correcta sigue enfrentando varios problemas no resueltos de forma satisfactoria. El primero es el muestreo eficiente de determinadas trayectorias de luz: los llamados caminos especular-difuso-especular (SDS), responsables de cáusticas vistas a través de reflejos u objetos refractantes complejos, son notoriamente difíciles de encontrar mediante muestreo aleatorio puro y siguen motivando variantes especializadas como el transporte de luz de Metropolis o las extensiones bidireccionales mencionadas en la sección 3.3.

Un segundo desafío es el compromiso entre sesgo y varianza que atraviesa buena parte de los algoritmos descritos: técnicas como el mapeo de fotones, el caché de irradiancia o los métodos de reducción de ruido basados en aprendizaje profundo introducen cierto sesgo -resultados sistemáticamente desviados del valor físicamente correcto- a cambio de imágenes visualmente limpias con menos cómputo; calibrar ese compromiso sin introducir artefactos perceptibles continúa siendo, en gran medida, un problema abierto.

En el ámbito de las aplicaciones interactivas, la geometría dinámica representa una dificultad persistente: las técnicas que dependen de un precálculo -la radiosidad horneada, la transferencia de radiancia precalculada- pierden su principal ventaja en escenas donde los objetos o la iluminación cambian de forma impredecible, y aunque el trazado de rayos acelerado por hardware ha reducido esta brecha, sigue exigiendo una capacidad de cómputo que no está disponible de manera uniforme en todas las plataformas, particularmente en dispositivos móviles.

Finalmente, los métodos de renderizado neuronal descritos en la sección 3.6, pese a su rapidez y fidelidad fotográfica, aún tienen dificultades para generalizar a condiciones de iluminación distintas de las observadas durante el entrenamiento -el problema del relighting- y para integrarse de manera fluida con los flujos de trabajo de edición de iluminación propios de la producción tradicional, lo que limita por ahora su adopción a casos de uso específicos en lugar de un reemplazo general de los métodos físicamente basados.

# **7. Conclusiones**

Los modelos globales de iluminación constituyen el conjunto de técnicas que permite a la computación gráfica reproducir el comportamiento real de la luz, incorporando la iluminación indirecta que los modelos locales no logran capturar. Su importancia radica en que efectos como las sombras suaves, el sangrado de color, las cáusticas y la interreflexión difusa son esenciales para alcanzar un realismo convincente.

Del análisis se desprende que no existe un algoritmo universalmente superior, sino que la elección depende del equilibrio buscado entre realismo físico, costo computacional y restricciones de tiempo. La radiosidad sobresale en la iluminación difusa de escenas estáticas; el trazado de rayos aporta reflejos y refracciones precisos; el trazado de trayectorias ofrece una solución unificada y físicamente correcta a costa de tiempos elevados; el mapeo de fotones resuelve con eficiencia las cáusticas y los materiales translúcidos; y el renderizado neuronal ofrece una vía alternativa, basada en datos, para reconstruir escenas reales con una fidelidad fotográfica difícil de igualar por los métodos puramente físicos. En la práctica, las soluciones más potentes combinan varios de estos métodos.

Como se discutió en la sección 6, persisten además varios desafíos no resueltos -el muestreo de caminos de luz complejos, el compromiso entre sesgo y varianza, la geometría dinámica y la generalización de los métodos neuronales a nuevas condiciones de iluminación- que continúan orientando buena parte de la investigación actual en el campo.

Finalmente, la evolución reciente del campo, impulsada por la aceleración por hardware y por el aprendizaje profundo, ha acercado la iluminación global físicamente correcta a las aplicaciones interactivas, antes reservada al cálculo diferido. Todo apunta a que el futuro próximo estará marcado por enfoques híbridos y por el uso de redes neuronales para reconstruir y acelerar el transporte de la luz, difuminando aún más la frontera entre el renderizado en tiempo real y el renderizado de producción (Aguirre Pérez, 2024; Gutiérrez Rodríguez, 2022; Kerbl et al., 2023).

**Referencias**

Aguirre Pérez, F. (2024). *Fingxels: Algoritmo de iluminación global basado en el uso de vóxeles* [Proyecto de grado, Universidad de la República, Uruguay]. Repositorio COLIBRI. https://hdl.handle.net/20.500.12008/48415

Cook, R. L., Porter, T., & Carpenter, L. (1984). Distributed ray tracing. *Computer Graphics (SIGGRAPH '84 Proceedings), 18*(3), 137-145.

Cook, R. L., & Torrance, K. E. (1982). A reflectance model for computer graphics. *ACM Transactions on Graphics, 1*(1), 7-24.

Departamento de Informática. (s.f.). *Modelos de iluminación global* [Material del curso Ampliación de Informática Gráfica]. Universidad de Valencia. https://informatica.uv.es/iiguia/AIG/web\_teoria/Tema4\_bn.pdf

Goral, C. M., Torrance, K. E., Greenberg, D. P., & Battaile, B. (1984). Modeling the interaction of light between diffuse surfaces. *Computer Graphics (SIGGRAPH '84 Proceedings), 18*(3), 213-222.

Gutiérrez Rodríguez, C. I. (2022). *Técnicas de aprendizaje profundo para el desarrollo de iluminación global en tiempo real* [Tesis de maestría, Universidad de la República, Uruguay]. Repositorio COLIBRI.

Hachisuka, T., Ogaki, S., & Jensen, H. W. (2008). Progressive photon mapping. *ACM Transactions on Graphics (SIGGRAPH Asia 2008), 27*(5), Artículo 130.

Jensen, H. W. (1996). Global illumination using photon maps. En *Rendering Techniques '96* (pp. 21-30). Springer.

Kajiya, J. T. (1986). The rendering equation. *Computer Graphics (SIGGRAPH '86 Proceedings), 20*(4), 143-150.

Keller, A. (1997). Instant radiosity. *Proceedings of SIGGRAPH '97*, 49-56.

Kerbl, B., Kopanas, G., Leimkühler, T., & Drettakis, G. (2023). 3D Gaussian splatting for real-time radiance field rendering. *ACM Transactions on Graphics, 42*(4), Artículo 139.

Landis, H. (2002). *Production-ready global illumination* [Notas del curso 16]. ACM SIGGRAPH 2002 Course Notes.

Machado de Benedetti, M. (2021). *Renderizado en tiempo real acelerado mediante hardware de ray tracing* [Proyecto de grado, Universidad de la República, Uruguay]. Repositorio COLIBRI.

Mildenhall, B., Srinivasan, P. P., Tancik, M., Barron, J. T., Ramamoorthi, R., & Ng, R. (2020). NeRF: Representing scenes as neural radiance fields for view synthesis. En A. Vedaldi, H. Bischof, T. Brox, & J. Frahm (Eds.), *Computer Vision – ECCV 2020* (pp. 405-421). Springer.

Sloan, P.-P., Kautz, J., & Snyder, J. (2002). Precomputed radiance transfer for real-time rendering in dynamic, low-frequency lighting environments. *ACM Transactions on Graphics, 21*(3), 527-536.

Walter, B., Marschner, S. R., Li, H., & Torrance, K. E. (2007). Microfacet models for refraction through rough surfaces. En *Rendering Techniques 2007: Proceedings of the 18th Eurographics Conference on Rendering* (pp. 195-206). Eurographics Association.

Ward, G. J., Rubinstein, F. M., & Clear, R. D. (1988). A ray tracing solution for diffuse interreflection. *Computer Graphics (SIGGRAPH '88 Proceedings), 22*(4), 85-92.

Whitted, T. (1980). An improved illumination model for shaded display. *Communications of the ACM, 23*(6), 343-349.

Zhukov, S., Iones, A., & Kronin, G. (1998). An ambient light illumination model. En *Rendering Techniques '98: Proceedings of the Eurographics Workshop on Rendering* (pp. 45-56). Springer.

**Anexo: Indicaciones para el Programa de Demostración**

Como complemento de este informe y de la exposición oral, el equipo desarrollará un programa que permita mostrar y poner a prueba, de manera práctica, los conceptos descritos en las secciones anteriores. Esta sección no forma parte del marco teórico del trabajo, sino que recoge un conjunto de indicaciones orientadas a guiar su diseño e implementación.

## **A.1. Objetivo del Programa**

El programa debe permitir comparar visualmente, sobre una misma escena, el comportamiento de al menos dos o tres de los modelos descritos -idealmente uno representativo de la iluminación local, uno de la iluminación global offline (trazado de rayos, radiosidad, trazado de trayectorias o mapeo de fotones) y uno de las aproximaciones en tiempo real-, de modo que la audiencia pueda apreciar las diferencias de calidad, tiempo de cómputo y comportamiento ante cada efecto (sombras suaves, sangrado de color, cáusticas, ruido) discutidas en el informe.

## **A.2. Escena de Prueba Recomendada**

Se recomienda utilizar la caja de Cornell, ya mencionada en la sección 3.2, como escena base para todas las pruebas. Al tratarse de una escena estándar en la investigación de iluminación global, con paredes de colores saturados, permite apreciar con claridad el sangrado de color, las sombras suaves y la interreflexión difusa, y existen imágenes de referencia publicadas con las que se puede contrastar el resultado propio. Usar la misma escena en todas las demostraciones, en lugar de una distinta por integrante, facilita además la comparación directa entre técnicas durante la exposición.

## **A.3. Posibles Plataformas de Desarrollo**

La elección de la plataforma depende del tiempo disponible y de la experiencia previa del equipo en programación gráfica. La Tabla 4 resume las alternativas más razonables.

**Tabla 4**

*Alternativas de plataforma para el programa de demostración*

| **Opción** | **Tecnología** | **Ventajas** | **Limitaciones** |
| --- | --- | --- | --- |
| Motor de juego | Unity (HDRP) o Unreal Engine 5 (Lumen) | Resultados visuales convincentes con poco código; permite alternar técnicas en tiempo real con un clic | Curva de aprendizaje del motor; menos control sobre el algoritmo interno |
| Renderizador propio | Python + NumPy (CPU) o C++ con OpenGL/Vulkan | Control total del algoritmo; útil para mostrar el funcionamiento interno del trazado de rayos o el path tracer | Tiempos de render lentos en CPU; requiere más tiempo de desarrollo |
| Web interactivo | three.js / WebGL o WebGPU | Se ejecuta en cualquier navegador, ideal para la exposición sin instalar nada; fácil de compartir | Implementar radiosidad o mapeo de fotones completos es más laborioso |
| Software existente | Blender (Cycles vs. Eevee Next) | Configuración rápida, sin programar; resultados profesionales inmediatos | Demuestra el efecto, no el algoritmo; menos “propio” como proyecto de programación |

## **A.4. Funcionalidades Sugeridas por Integrante**

Para mantener la coherencia con la distribución de la Tabla 1, se sugiere que cada integrante implemente o demuestre la parte del programa correspondiente a su bloque temático:

* **Integrante 1:** un interruptor que alterne entre un término ambiente constante (iluminación local) y una aproximación de iluminación indirecta, para visualizar directamente la diferencia conceptual de la sección 2.1.
* **Integrante 2:** un trazador de rayos recursivo (estilo Whitted) con reflejos, refracciones y sombras; opcionalmente, una solución de radiosidad simplificada sobre la caja de Cornell que muestre el sangrado de color.
* **Integrante 3:** un trazador de trayectorias progresivo, con un control deslizante de muestras por píxel y un contador visible, para mostrar en vivo cómo disminuye el ruido al aumentar las muestras; opcionalmente, cáusticas simples mediante mapeo de fotones.
* **Integrante 4:** alternar oclusión ambiental y/o reflejos trazados por rayos sobre un render base rasterizado, mostrando el costo en fotogramas por segundo de cada efecto, y un panel final que resuma los tiempos y la calidad obtenidos por cada técnica implementada por el equipo.

## **A.5. Métricas a Registrar**

Para reforzar empíricamente la Tabla 2 y la Tabla 3 del informe, conviene registrar durante las pruebas del programa al menos: el tiempo de render por imagen o por fotograma, el número de muestras o fotones necesarios para alcanzar un resultado visualmente limpio, y -en las técnicas en tiempo real- los fotogramas por segundo con y sin la técnica activada. Estos datos, presentados como una tabla o un gráfico breve dentro de la propia exposición, le dan al trabajo un componente cuantitativo propio que complementa la revisión bibliográfica.

## **A.6. Recomendaciones Prácticas para la Demostración en Vivo**

Dado que algunas de estas técnicas -en particular el trazado de trayectorias y el mapeo de fotones- pueden tardar varios segundos o minutos en converger, se recomienda no depender exclusivamente del render en vivo durante la exposición: conviene tener capturas de pantalla o un video corto pregrabado como respaldo para cada demostración, y reservar la ejecución en vivo para los casos en los que el tiempo de respuesta sea lo suficientemente rápido (por ejemplo, alternar oclusión ambiental o cambiar el número de muestras de un path tracer ya optimizado). De esta manera se evita que un imprevisto técnico interrumpa el flujo de la presentación.