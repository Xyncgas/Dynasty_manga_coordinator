#pragma once
//a block for get_random_base64
#if 1
std::array<char, 64> base64_dictionary_apx333 = { 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/' };
std::string get_random_base64(int counts/*how many letter do you need huh*/)
{
	//srand(time(NULL)); - doing it in entry point - otherwise for calls being so close together you might be getting same random numbers since time(NULL) is in seconds that means every calls in same second would produce the same random number and instead the correct way is to initiate once and call rand afterwards and everytime no matter how frequent you are calling the result is expected to be different and remember rand can't be threaded or data race...

	std::string output; output.reserve(counts);
	for (int i = 0; i < counts; i++)//10 random letter
	{
		int pos = rand() % 63 + 0;
		output += base64_dictionary_apx333[pos];
	}
	return output;
}
#endif
//a block for get_random_base64_url_standard
#if 1
std::array<char, 62> base64_url_standard_dictionary_apx333 = { 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z','0','1','2','3','4','5','6','7','8','9' };
std::string get_random_base64_url_standard(int counts/*how many letter do you need huh*/)
{
	//srand(time(NULL)); - doing it in entry point - otherwise for calls being so close together you might be getting same random numbers since time(NULL) is in seconds that means every calls in same second would produce the same random number and instead the correct way is to initiate once and call rand afterwards and everytime no matter how frequent you are calling the result is expected to be different and remember rand can't be threaded or data race...

	std::string output; output.reserve(counts);//counts large might cause bad alloc to throw on output.reserve
	for (int i = 0; i < counts; i++)//10 random letter
	{
		int pos = rand() % 61 + 0;
		output += base64_url_standard_dictionary_apx333[pos];
	}
	return output;
}
#endif


std::string react_0mjo7xS8dkiTI(std::string modes/*url or plain*/, int counts)
{
	if (modes == "url")
	{
		return get_random_base64_url_standard(counts);
	}
	else
	{
		return get_random_base64(counts);
	}
}