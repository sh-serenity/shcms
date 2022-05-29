pipeline { 
        environment { 
        registry = "stormstack/bscms" 
        registryCredential = '1qaz2wsx3edc' 
        dockerImage = '' 
    }
    agent any 
    stages { 
        stage('Cloning our Git') { 
            steps {
                git clone 'https://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@github.com/sh-serenity/shcms.git' 
            }
        } 
        stage('Building our image') { 
            steps { 
                script { 
                    dockerImage = docker.build registry + ":$BUILD_NUMBER" 
                }
            } 
        }
        stage('Deploy our image') { 
            steps { 
                script { 
                    docker.withRegistry( '', registryCredential ) { 
                        dockerImage.push() 
                    }
                } 
            }
        } 
        stage('Cleaning up') { 
            steps { 
                sh "docker rmi $registry:$BUILD_NUMBER" 
            }
        } 
    }
}

